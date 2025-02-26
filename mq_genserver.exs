defmodule MQ2ElixirServer do
  use GenServer

  @port 4000  # Port to listen on
  defstruct characters: %{}, clients: %{}
  # Starts the GenServer
  def start_link(_) do
    GenServer.start_link(__MODULE__, %{}, name: __MODULE__)
  end

  def init(state) do
    {:ok, socket} = :gen_tcp.listen(@port, [:binary, packet: 0, active: false, reuseaddr: true])
    spawn(fn -> accept_connections(socket) end)
    IO.puts("MQ2ElixirServer started on port #{@port}")
    {:ok, %{characters: %{}, clients: %{}}}
  end

  #############################################
  ## GenServer API
  #############################################
  def register_character(name), do: GenServer.cast(__MODULE__, {:register_character, name})
  def get_state(name), do: GenServer.call(__MODULE__, {:get_state, name})

  def update_character(name, updates) do
    GenServer.cast(__MODULE__, {:update_character, name, updates})
  end

  #############################################
  ## GenServer Callbacks
  #############################################
  def handle_cast({:update_character, name, updates}, state) do
    new_state = Map.update(state, name, %{}, &Map.merge(&1, updates))
    {:noreply, new_state}
  end


def handle_cast({:register_character, raw_name, socket}, state) when is_binary(raw_name) and is_port(socket) do
  # Split by whitespace/newline and take the first word (assuming it's the character name)
  clean_name = 
    raw_name
    |> String.split(~r/\s+/, trim: true)  # Split on spaces/newlines
    |> List.first()                       # Take only the first valid name
    |> String.replace(~r/\bSTATE\b/, "")
  IO.puts("Registering character: #{clean_name}")

  new_characters = Map.put(state.characters, clean_name, %{level: 1, hp: 100})
  new_clients = Map.put(state.clients, clean_name, socket)

  new_state = %{state | characters: new_characters, clients: new_clients}

  broadcast_message("#{clean_name} has joined the server.", new_state.clients)

  IO.puts("After register: #{inspect(new_state)}")
  {:noreply, new_state}
end



def handle_call({:get_state, name}, _from, state) do
    {:reply, Map.get(state, name, %{}), state}
  end

  defp accept_connections(socket) do
    {:ok, client} = :gen_tcp.accept(socket)
    IO.puts("New client connected.")
    spawn(fn -> client_loop(client) end)  # Immediately start handling
    accept_connections(socket)
  end

  #############################################
  ## Private helper functions
  #############################################

defp client_loop(socket) do
  case :gen_tcp.recv(socket, 0) do
    {:ok, data} ->
      message = String.trim(data)  # Trim any newline characters
      IO.puts("Received raw message: #{inspect(data)}")  # Debug raw data
      IO.puts("Processed message: #{inspect(message)}")  # Debug cleaned message

      case process_command(message, socket) do
        "DISCONNECT" ->
          IO.puts("Client requested disconnect.")
          :gen_tcp.send(socket, "Goodbye!\n")
          :gen_tcp.close(socket)

        response ->
          :gen_tcp.send(socket, response <> "\n")
          client_loop(socket)  # Keep listening for more messages
      end

    {:error, :closed} ->
      IO.puts("Client disconnected.")
  end
end


defp process_command(message, socket) do
  IO.puts("Processing command: #{inspect(message)}")

  case String.split(message, " ", parts: 2) do
    ["REGISTER", character] ->
      clean_character = String.trim(character) |> String.replace("\n", "")
      IO.puts("Attempting to register character: #{inspect(clean_character)}")
      GenServer.cast(__MODULE__, {:register_character, clean_character, socket})
      "Character registered: #{clean_character}"

    _ ->
      IO.puts("Unknown command received")
      "Unknown command"
  end
end


#  defp process_command(message) do
#    IO.puts("Processing command: #{message}")
#
#    case String.split(message, " ") do
#      ["REGISTER", character] ->
#        GenServer.cast(__MODULE__, {:register_character, character})
#        "Character registered: #{character}"
#
#      ["STATE", character] ->
#       state = GenServer.call(__MODULE__, {:get_state, character})
#       "State for #{character}: #{inspect(state)}"
#
#     _ -> "Unknown command"
#   end
# end

defp broadcast_message(message, clients) do
  Enum.each(clients, fn {_name, socket} ->
    if socket != nil do
      :gen_tcp.send(socket, "Broadcast: #{message}\n")
    else
      IO.puts("Warning: Nil socket detected, skipping.")
    end
  end)
end


end

# Explicitly start the GenServer and prevent the script from exiting
{:ok, _pid} = MQ2ElixirServer.start_link([])
Process.sleep(:infinity)
