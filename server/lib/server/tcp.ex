defmodule ElixirTcpServer.TCP do
  @moduledoc false
  use GenServer
  require Logger

  alias ElixirTcpServer.ClientRegistry

  @port 4000

  # ────────── public bootstrap ──────────
  def start_link(_args),
    do: GenServer.start_link(__MODULE__, [], name: __MODULE__)

  # ────────── server lifecycle ──────────
  @impl true
  def init(_) do
    {:ok, listener} =
      :gen_tcp.listen(@port, [:binary, packet: :line, active: false, reuseaddr: true])

    Logger.info("✅ Listening on port #{@port}")
    spawn_link(fn -> accept_loop(listener) end)
    {:ok, []}
  end

  # ────────── accept / handshake ──────────
  defp accept_loop(listener) do
    {:ok, socket} = :gen_tcp.accept(listener)
    Logger.info("🔌 Client connected")
    Task.start_link(fn -> handle_client(socket) end)
    accept_loop(listener)
  end

  defp handle_client(socket) do
    case :gen_tcp.recv(socket, 0) do
      {:ok, raw} ->
        trimmed = String.trim(raw)

        with {:ok, %{"character" => %{"name" => name, "active" => active}}} <-
               Jason.decode(trimmed),
             false <- invalid_name?(name) do
          Logger.info("🟢 Registered #{name} (active=#{active})")
          :gen_tcp.send(socket, "[Server] Welcome #{name}\n")
          ClientRegistry.add(name, socket, active)
          client_loop(socket, name)
        else
          true ->
            Logger.warn("⚠️ Invalid character name")
            :gen_tcp.send(socket, "[Server] Invalid character name\n")
            :gen_tcp.close(socket)

          {:ok, _other} ->
            Logger.warn("⚠️ Unexpected registration: #{trimmed}")
            :gen_tcp.send(socket, "[Server] Invalid JSON\n")
            :gen_tcp.close(socket)

          {:error, _} ->
            Logger.error("❌ Failed to receive registration")
            :gen_tcp.close(socket)
        end

      {:error, _} ->
        Logger.error("❌ Error on initial recv")
        :gen_tcp.close(socket)
    end
  end

  # ────────── per-client loop ──────────
  defp client_loop(socket, name) do
    case :gen_tcp.recv(socket, 0) do
      {:ok, raw} ->
        msg = String.trim(raw)
        Logger.debug("📨 #{name}: #{msg}")

        cond do
          msg == "ping" ->
            :gen_tcp.send(socket, "pong\n")

          msg == "names" ->
            roster =
              ClientRegistry.list()
              |> Enum.map(fn
                {n, %{active: true}} -> "*#{n}*"
                {n, _} -> n
              end)
              |> Enum.join(", ")

            :gen_tcp.send(socket, "[Server] Clients: #{roster}\n")

          msg == "status active" ->
            Logger.info("⭐️ #{name} claims ACTIVE")
            ClientRegistry.set_active(name)

          msg == "status inactive" ->
            Logger.info("🔕 #{name} now inactive")
            ClientRegistry.update_active(name, false)

          String.starts_with?(msg, "all cometome ") ->
            # e.g. "all cometome Sandayar"
            Logger.info("📢 Come‐to‐me request: #{msg}")
            # prefix so plugin sees “cmd all cometome Sandayar”
            ClientRegistry.broadcast_all("cmd " <> msg)



          String.starts_with?(msg, "cmd all ") ->
            Logger.info("📢 Broadcast-ALL: #{msg}")
            ClientRegistry.broadcast_all(msg)

          String.starts_with?(msg, "cmd ") ->
            Logger.info("📢 Broadcast: #{msg}")
            ClientRegistry.broadcast_except_active(msg)

          true ->
            Logger.warn("⚠️ Unknown message from #{name}: #{msg}")
        end

        client_loop(socket, name)

      {:error, :closed} ->
        Logger.info("❌ #{name} disconnected")
        ClientRegistry.remove(name)

      {:error, reason} ->
        Logger.error("❌ recv error from #{name}: #{inspect(reason)}")
        :gen_tcp.close(socket)
        ClientRegistry.remove(name)
    end
  end

  # ────────── helpers ──────────
  defp invalid_name?(n), do: String.contains?(n, " ") or byte_size(n) > 20
end
