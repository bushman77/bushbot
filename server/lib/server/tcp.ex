# lib/elixir_tcp_server/tcp.ex
defmodule TCP do
  use GenServer
  require Logger

  @port               4000
  @heartbeat_interval 1_000

  # ──────────────────────────────────────────────────────────────
  # Public API
  # ──────────────────────────────────────────────────────────────
  def start_link(_), do: GenServer.start_link(__MODULE__, %{}, name: __MODULE__)

  # ──────────────────────────────────────────────────────────────
  # GenServer callbacks
  # ──────────────────────────────────────────────────────────────
  @impl true
  def init(_state) do
    {:ok, listener} =
      :gen_tcp.listen(@port, [:binary, packet: :line, active: false, reuseaddr: true])

    Logger.info("✅ Listening on port #{@port}")
    send(self(), :accept)
    {:ok, %{listener: listener}}
  end

  @impl true
  def handle_info(:accept, %{listener: listener} = state) do
    {:ok, socket} = :gen_tcp.accept(listener)
    peer = peername(socket)
    Logger.info("🔌 Client connected: #{peer}")

    :inet.setopts(socket, [keepalive: true, active: :once])
    ClientRegistry.add(peer, socket, false)

    Process.send_after(self(), {:heartbeat, socket, peer}, @heartbeat_interval)
    send(self(), :accept)
    {:noreply, state}
  end

  @impl true
  # All incoming TCP data lands here and is routed by handle_command/3
  def handle_info({:tcp, socket, raw}, state) do
IO.inspect %{stepOne: raw, socket: socket}
    peer    = peername(socket)
    raw_msg = String.trim(raw)
    msg     = Regex.replace(~r/^\[[^\]]+\]\s*/, raw_msg, "")
IO.inspect :stepTwo
    Logger.debug("📥 [#{peer}] #{inspect(msg)}")
    handle_command(socket, peer, msg)
    :inet.setopts(socket, active: :once)
    {:noreply, state}
  end

  @impl true
  def handle_info({:tcp_closed, socket}, state) do
    ClientRegistry.remove(peername(socket))
    {:noreply, state}
  end
  @impl true
  def handle_info({:tcp_error, socket, reason}, state) do
    Logger.error("🚨 TCP error from #{peername(socket)}: #{inspect(reason)}")
    ClientRegistry.remove(peername(socket))
    {:noreply, state}
  end

  @impl true
  def handle_info({:heartbeat, socket, peer}, state) do
    case :gen_tcp.send(socket, "ping\n") do
      :ok ->
        Logger.debug("💓 Sent ping to #{peer}")
        Process.send_after(self(), {:heartbeat, socket, peer}, @heartbeat_interval)

      {:error, _} = err ->
        Logger.warn("⚠️ Heartbeat failed for #{peer}: #{inspect(err)}, removing client")
        ClientRegistry.remove(peer)
    end
    {:noreply, state}
  end

  # ──────────────────────────────────────────────────────────────
  # Command router
  # ──────────────────────────────────────────────────────────────
  defp handle_command(_socket, _peer, ""),                            do: :ok
  defp handle_command(_socket, peer, "ping"),                       do: Logger.debug("[#{peer}] pong")
  defp handle_command(socket, _peer, "{" <> _ = json),              do: handle_registration(json, socket)
  defp handle_command(_socket, peer, "status active"),              do: ClientRegistry.update_active(peer, true)
  defp handle_command(_socket, peer, "status inactive"),            do: ClientRegistry.update_active(peer, false)

  defp handle_command(_socket, _peer, "cmd switch " <> target) do
    Logger.info("🔄 Switching active client to #{target}")
    ClientRegistry.update_active(target, true)
    ClientRegistry.route_to(target, "cmd switch #{target}")
  end

  defp handle_command(_socket, _peer, "cmd all " <> cmd) do
    ClientRegistry.broadcast_to_inactive(cmd)
  end

  defp handle_command(_socket, _peer, "cmd " <> rest) do
    case String.split(rest, " ", parts: 2) do
      [target, action] ->
        ClientRegistry.route_to(target, action)
      _ ->
        Logger.warn("⚠️ Malformed cmd: #{rest}")
    end
  end

  defp handle_command(_socket, peer, other) do
    Logger.warn("❓ Unhandled message from #{peer}: #{inspect(other)}")
  end

  # ──────────────────────────────────────────────────────────────
  # Helpers
  # ──────────────────────────────────────────────────────────────
  def peername(socket) do
    case :inet.peername(socket) do
      {:ok, {ip, port}} -> "#{:inet.ntoa(ip)}:#{port}"
      _                 -> "unknown"
    end
  end

  defp handle_registration(json, socket) do
    case Jason.decode(json) do
      {:ok, %{"character" => c}} ->
        name   = c["name"]
        active = !!c["active"]
        ClientRegistry.add(name, socket, active)
        ClientRegistry.remove(peername(socket))
      {:error, err} ->
        Logger.error("🚨 JSON decode error for #{peername(socket)}: #{inspect(err)}")
    end
  end
end
