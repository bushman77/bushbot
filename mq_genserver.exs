defmodule MQ2ElixirServer do
  use GenServer

  @port 4000  # Port to listen on

  defstruct socket: nil, clients: %{}

  def start_link(_) do
    GenServer.start_link(__MODULE__, %{}, name: __MODULE__)
  end

  def init(_) do
    {:ok, listener_socket} =
      :gen_tcp.listen(@port, [:binary, packet: 0, active: false, reuseaddr: true])

    spawn(fn -> accept_connections(listener_socket) end)
    IO.puts("MQ2ElixirServer started on port #{@port}")

    {:ok, %{socket: listener_socket, clients: %{}}}
  end

  #############################################
  ## GenServer API
  #############################################

  def register_character(name, socket), do: GenServer.cast(__MODULE__, {:register_character, name, socket})
  def get_state(name), do: GenServer.call(__MODULE__, {:get_state, name})
  def update_character(name, updates), do: GenServer.cast(__MODULE__, {:update_character, name, updates})
  def send_command(from, to, command), do: GenServer.cast(__MODULE__, {:send_command, from, to, command})

  #############################################
  ## GenServer Callbacks
  #############################################

  def handle_call({:get_state, name}, _from, state) do
    {:reply, Map.get(state.clients, name, %{}), state}
  end

  def handle_cast({:register_character, name, socket}, state) do
    if Map.has_key?(state.clients, name) do
      :gen_tcp.send(socket, "Error: Character #{name} is already registered.\n")
      {:noreply, state}
    else
      new_clients = Map.put(state.clients, name, %{level: 1, hp: 100, socket: socket})

      IO.puts("#{name} has joined the server.")
      {:noreply, %{state | clients: new_clients}}
    end
  end

  def handle_cast({:remove_client, socket}, state) do
    {disconnected_character, _} =
      Enum.find(state.clients, fn {_name, data} -> data.socket == socket end) || {nil, nil}

    new_clients = Map.drop(state.clients, [disconnected_character])

    case disconnected_character do
      nil -> :ok
      character -> IO.puts("#{character} disconnected.")
    end
    {:noreply, %{state | clients: new_clients}}
  end

  def handle_cast({:update_character, name, updates}, state) do
    new_clients = Map.update(state.clients, name, %{}, &Map.merge(&1, updates))
    {:noreply, %{state | clients: new_clients}}
  end

  def handle_cast({:send_command, from, to, command}, state) do
    if Map.has_key?(state.clients, to) do
      target_socket = state.clients[to].socket
      :gen_tcp.send(target_socket, "#{from} commands you: #{command}\n")
    else
      IO.puts("Failed to send command: #{to} is not online.")
    end

    {:noreply, state}
  end

  #############################################
  ## Socket Handling
  #############################################

  def accept_connections(listener_socket) do
    case :gen_tcp.accept(listener_socket) do
      {:ok, socket} ->
        IO.puts("New client connected.")
        spawn(fn -> client_loop(socket) end)
        accept_connections(listener_socket)

      {:error, reason} ->
        IO.puts("Error accepting connection: #{inspect(reason)}")
        Process.sleep(1000)
        accept_connections(listener_socket)
    end
  end

  def client_loop(socket) do
    case :gen_tcp.recv(socket, 0) do
      {:ok, data} ->
        message = String.trim(data)
        IO.puts("Received message: #{inspect(message)}")

        case process_command(message, socket) do
          "DISCONNECT" ->
            IO.puts("Client requested disconnect.")
            GenServer.cast(__MODULE__, {:remove_client, socket})

          response ->
            :gen_tcp.send(socket, response <> "\n")
            client_loop(socket)
        end

      {:error, :closed} ->
        IO.puts("Client disconnected.")
        GenServer.cast(__MODULE__, {:remove_client, socket})
    end
  end

  def process_command(message, socket) do
    IO.puts("Processing command: #{inspect(message)}")

    case String.split(message, " ", parts: 3) do
      ["REGISTER", character] when is_binary(character) and character != "" ->
        clean_character = String.trim(character)
        GenServer.cast(__MODULE__, {:register_character, clean_character, socket})
        "Character registered: #{clean_character}"

      ["STATE", character] ->
        state = GenServer.call(__MODULE__, {:get_state, character})
        if state == %{} do
          "Character not found: #{character}"
        else
          "State for #{character}: #{inspect(state)}"
        end

      ["COMMAND", from, to, command] ->
        GenServer.cast(__MODULE__, {:send_command, from, to, command})
        "Command sent to #{to}: #{command}"

      _ ->
        IO.puts("Unknown command received: #{inspect(message)}")
        "Unknown command"
    end
  end
end

# Start the server
{:ok, _pid} = MQ2ElixirServer.start_link([])
Process.sleep(:infinity)
