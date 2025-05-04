defmodule ElixirTcpServer.TCP do
  @moduledoc false
  use GenServer
  require Logger

  alias ElixirTcpServer.ClientRegistry

  @port 4000

  # ──────────────────────────────────────────────────────────────
  # Public API
  # ──────────────────────────────────────────────────────────────

  def start_link(_args), do: GenServer.start_link(__MODULE__, [], name: __MODULE__)

  # ──────────────────────────────────────────────────────────────
  # GenServer callbacks
  # ──────────────────────────────────────────────────────────────

  @impl true
  def init(_) do
    {:ok, listener} =
      :gen_tcp.listen(@port, [:binary, packet: :line, active: false, reuseaddr: true])

    Logger.info("✅ Listening on port #{@port}")
    spawn(fn -> accept_loop(listener) end)
    {:ok, []}
  end

  # ──────────────────────────────────────────────────────────────
  # Connection handling
  # ──────────────────────────────────────────────────────────────

  defp accept_loop(listener) do
    {:ok, socket} = :gen_tcp.accept(listener)
    {:ok, {ip, port}} = :inet.peername(socket)
    name = "#{:inet.ntoa(ip)}:#{port}"

    Logger.info("🔌 #{name} connected")
    ClientRegistry.add(name, socket, false)

    # hand off to client loop
    spawn(fn -> client_loop(socket, name) end)

    accept_loop(listener)
  end

  defp client_loop(socket, name) do
    case :gen_tcp.recv(socket, 0) do
      {:ok, data} ->
        msg = String.trim_trailing(data, "\n")
        handle_message(msg, socket, name)
        client_loop(socket, name)

      {:error, :closed} ->
        Logger.info("❌ #{name} disconnected")
        ClientRegistry.remove(name)
        :ok

      {:error, reason} ->
        Logger.error("❌ recv error from #{name}: #{inspect(reason)}")
        ClientRegistry.remove(name)
        :ok
    end
  end

  # ──────────────────────────────────────────────────────────────
  # Message dispatch
  # ──────────────────────────────────────────────────────────────

  # ECA command → forward as "cmd …"
  defp handle_message("/eca " <> rest, socket, name) do
    command = "cmd " <> rest
    ClientRegistry.broadcast_all(command)
    Logger.info("✉️ #{name} → #{command}")
    :ok
  end

  # Example: handle "names" request
  defp handle_message("names", socket, name) do
    names = ClientRegistry.list_names()
    :gen_tcp.send(socket, Enum.join(names, ",") <> "\n")
    :ok
  end

  # Always accept pong
  defp handle_message("pong", _socket, _name) do
    :ok
  end

  # Fallback for anything else
  defp handle_message(msg, _socket, name) do
    Logger.warning("⚠️ Unknown message from #{name}: #{msg}")
    :ok
  end
end
