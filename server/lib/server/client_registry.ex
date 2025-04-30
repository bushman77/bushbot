defmodule ElixirTcpServer.ClientRegistry do
  use Agent

  def start_link(_), do: Agent.start_link(fn -> %{} end, name: __MODULE__)

  def add(name, socket) do
    Agent.update(__MODULE__, fn clients -> Map.put(clients, socket, name) end)
  end

  def remove(socket) do
    Agent.update(__MODULE__, fn clients -> Map.delete(clients, socket) end)
  end

  def get_name(socket) do
    Agent.get(__MODULE__, fn clients -> Map.get(clients, socket, "unknown") end)
  end

  # ✅ Broadcast from a client
  def broadcast(from_socket, message) do
    from_name = get_name(from_socket)

    Agent.get(__MODULE__, fn clients ->
      Enum.each(clients, fn {sock, _name} ->
        :gen_tcp.send(sock, "[#{from_name}] says: #{message}\n")
      end)
    end)
  end

  # ✅ Broadcast from system/admin
  def broadcast(message) do
    Agent.get(__MODULE__, fn clients ->
      Enum.each(clients, fn {sock, _name} ->
        :gen_tcp.send(sock, "[Server] #{message}\n")
      end)
    end)
  end
end
