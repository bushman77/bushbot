defmodule ElixirTcpServer.TCP do
  use GenServer

  def start_link(_args), do: GenServer.start_link(__MODULE__, [], name: __MODULE__)

  def init(_) do
    {:ok, socket} = :gen_tcp.listen(4000, [:binary, packet: :line, active: false, reuseaddr: true])
    IO.puts("✅ Listening on port 4000")
    spawn(fn -> accept_loop(socket) end)
    {:ok, []}
  end

  defp accept_loop(listener) do
    {:ok, socket} = :gen_tcp.accept(listener)
    IO.puts("🔌 Client connected")
    Task.start(fn -> handle_client(socket) end)
    accept_loop(listener)
  end

  defp handle_client(socket) do
    # First message must be the character name
    case :gen_tcp.recv(socket, 0) do
      {:ok, name_raw} ->
        name = String.trim(name_raw)

        if invalid_name?(name) do
          IO.puts("⚠️ Invalid character name: #{inspect(name)}")
          :gen_tcp.send(socket, "[Server] Invalid character name. Connection closed.\n")
          :gen_tcp.close(socket)
        else
          IO.puts("🟢 Registered client: #{name}")
          :gen_tcp.send(socket, "[Server] Welcome #{name}\n")
          ElixirTcpServer.ClientRegistry.add(name, socket)
          loop(socket)
        end

      {:error, _} ->
        IO.puts("❌ Failed to receive character name")
        :gen_tcp.send(socket, "[Server] Please identify yourself with a valid name.\n")
        :gen_tcp.close(socket)
    end
  end

  defp loop(socket) do
    case :gen_tcp.recv(socket, 0) do
      {:ok, data} ->
        msg = String.trim(data)
        IO.puts("📨 Received: #{msg}")

        case msg do
          "ping" ->
            :gen_tcp.send(socket, "pong\n")
          _ ->
            ElixirTcpServer.ClientRegistry.broadcast(socket, msg)
        end

        loop(socket)

      {:error, :closed} ->
        IO.puts("❌ Client disconnected")
        ElixirTcpServer.ClientRegistry.remove(socket)
    end
  end

  defp invalid_name?(name) do
    String.contains?(name, " ") or String.length(name) > 20 or name == "ping"
  end
end
