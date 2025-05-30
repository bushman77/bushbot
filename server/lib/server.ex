defmodule Server do
  use GenServer
  require Logger

  @port 4000
  @tcp_opts [:binary, packet: :line, active: false, reuseaddr: true]

  ## Public API

  def start_link(_opts \\ []) do
    GenServer.start_link(__MODULE__, %{}, name: __MODULE__)
  end

  def incoming_data(socket, data) do
    GenServer.cast(__MODULE__, {:incoming_data, socket, data})
  end

  def client_disconnected(socket) do
    GenServer.cast(__MODULE__, {:client_disconnected, socket})
  end

  ## GenServer Callbacks

  @impl true
  def init(_) do
    Character.start_link([])
    {:ok, listen_socket} = :gen_tcp.listen(@port, @tcp_opts)
    Logger.info("Listening on port #{@port}...")

    spawn(fn -> accept_loop(listen_socket) end)
    {:ok, %{listen_socket: listen_socket}}
  end

@impl true
def handle_cast({:incoming_data, socket, data}, state) do
  Logger.debug("Raw data received: #{inspect(data)}")

  case Jason.decode(data) do
    {:ok, %{"type" => "register", "character" => %{"name" => name} = char_info}} ->
      Character.set_character_info(socket, char_info)
      Logger.info("Registered: #{name}")
      :gen_tcp.send(socket, "Welcome #{name}! Registration successful.\n")

      Enum.each(Character.all(), fn 
        #if client != socket, do: :gen_tcp.send(client, "#{name} joined the server.\n")
        %{port: port} when port != socket ->
          :gen_tcp.send(port, "#{name} joined the server. \n")
        _ -> :ok
      end)

    {:ok, %{"type" => "cmd", "target" => target, "command" => command}} ->
      Logger.info("Command for #{target}: #{command}")
      :ok

    {:ok, other} ->
      Logger.warning("Unhandled message type: #{inspect(other)}")

    {:error, reason} ->
      Logger.error("Failed to decode JSON: #{inspect(reason)}")
  end

  {:noreply, state}
end

  @impl true
  def handle_cast({:client_disconnected, socket}, state) do
    Character.remove(socket)
    Logger.info("Client disconnected")
    {:noreply, state}
  end

  ## Private

  defp accept_loop(listen_socket) do
    {:ok, client} = :gen_tcp.accept(listen_socket)
    Logger.info("Client connected: #{inspect(client)}")
    Character.add(client)
    spawn(fn -> handle_client(client) end)
    accept_loop(listen_socket)
  end

  defp handle_client(socket) do
    case :gen_tcp.recv(socket, 0) do
      {:ok, data} ->
        Server.incoming_data(socket, data)
        handle_client(socket)

      {:error, :timeout} ->
        handle_client(socket)

      {:error, reason} ->
        Logger.warn("Socket error: #{inspect(reason)}")
        Server.client_disconnected(socket)
        :gen_tcp.close(socket)
    end
  end
end
