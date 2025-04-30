defmodule ElixirTcpServer do
  use GenServer

  def start_link(_), do: GenServer.start_link(__MODULE__, [], name: __MODULE__)

  def init(_) do
    Task.start(fn -> listen(4000) end)
    {:ok, []}
  end

  def listen(port) do
    {:ok, socket} = :gen_tcp.listen(port, [:binary, packet: :line, active: false, reuseaddr: true])
    IO.puts("Listening on port #{port}")
    loop_accept(socket)
  end

  defp loop_accept(socket) do
    {:ok, client} = :gen_tcp.accept(socket)
    GenServer.cast(__MODULE__, {:new_client, client})
    spawn(fn -> handle_client(client) end)
    loop_accept(socket)
  end

  def handle_cast({:new_client, socket}, state) do
    {:noreply, [socket | state]}
  end

  def handle_cast({:broadcast, message}, state) do
    Enum.each(state, fn sock ->
      :gen_tcp.send(sock, message <> "\n")
    end)
    {:noreply, state}
  end

  defp handle_client(socket) do
    case :gen_tcp.recv(socket, 0) do
      {:ok, data} ->
        msg = String.trim(data)
        IO.puts("Received: #{msg}")
        GenServer.cast(__MODULE__, {:broadcast, "[Elixir Relay] #{msg}"})
        handle_client(socket)

      {:error, :closed} ->
        IO.puts("Client disconnected")
    end
  end
end
