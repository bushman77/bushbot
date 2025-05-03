defmodule ElixirTcpServer.ClientRegistry do
  @moduledoc """
  In-memory registry of EverQuest clients.

  Stored per client:
    * `:socket` — TCP socket
    * `:active` — `true` if the window currently has focus

  Responsibilities
  • add / remove clients  
  • guarantee **exactly one** `active: true` at any time  
  • broadcast helpers
  • force commands to specific clients
  """

  use GenServer
  require Logger

  # ────────── Public API ──────────

  def start_link(_), do: GenServer.start_link(__MODULE__, %{}, name: __MODULE__)

  @doc """
  Adds a client to the registry with a given name, socket, and active status.
  """
  def add(name, socket, active \\ false) do
    :inet.setopts(socket, active: false)
    GenServer.cast(__MODULE__, {:add, name, socket, active})
  end

  @doc """
  Removes a client by name or socket, closing its connection.
  """
  def remove(name_or_socket), do: GenServer.cast(__MODULE__, {:remove, name_or_socket})

  @doc """
  Mark **exactly** `name` as active; all others become inactive.
  """
  def set_active(name), do: GenServer.cast(__MODULE__, {:set_active, name})

  @doc """
  Toggle a single client’s active flag without touching the rest.
  """
  def update_active(name, flag), do: GenServer.cast(__MODULE__, {:update_active, name, flag})

  @doc """
  Broadcast `msg` to every client **except** the active one.
  """
  def broadcast_except_active(msg), do: GenServer.cast(__MODULE__, {:broadcast_except_active, msg})

  @doc """
  Broadcast `msg` to **all** clients (active included).
  """
  def broadcast_all(msg), do: GenServer.cast(__MODULE__, {:broadcast_all, msg})

  @doc """
  Force a raw command to the named client, bypassing active-state checks.

  ## Examples
      iex> ClientRegistry.force_command("Bushman", "cmd sit")
  """
  def force_command(name, command) do
    GenServer.cast(__MODULE__, {:force_command, name, command})
  end

  @spec list() :: %{required(String.t()) => %{socket: port(), active: boolean()}}
  def list, do: GenServer.call(__MODULE__, :list)

  # ────────── GenServer callbacks ──────────

  @impl true
  def init(state), do: {:ok, state}

  @impl true
  def handle_cast({:add, name, socket, active}, state) do
    Logger.info("📘 add #{name} (active=#{active})")
    {:noreply, Map.put(state, name, %{socket: socket, active: active})}
  end

  @impl true
  def handle_cast({:remove, key}, state) do
    new_state =
      Enum.reduce(state, state, fn {name, %{socket: sock}}, acc ->
        if sock == key or name == key do
          Logger.info("🗑️ remove #{name}")
          safe_close(sock)
          Map.delete(acc, name)
        else
          acc
        end
      end)

    {:noreply, new_state}
  end

  @impl true
  def handle_cast({:set_active, name}, state) do
    if Map.has_key?(state, name) do
      Logger.info("🔄 set_active → #{name}")
      {:noreply, set_only_active(state, name)}
    else
      Logger.warn("⚠️ set_active: unknown #{name}")
      {:noreply, state}
    end
  end

  @impl true
  def handle_cast({:update_active, name, flag}, state) do
    case Map.fetch(state, name) do
      :error -> {:noreply, state}
      {:ok, entry} -> {:noreply, Map.put(state, name, %{entry | active: flag})}
    end
  end

  @impl true
  def handle_cast({:broadcast_except_active, msg}, state) do
    Enum.each(state, fn
      {_n, %{active: true}} -> :ok
      {_n, %{socket: s}} -> send_to_socket(s, msg)
    end)

    {:noreply, state}
  end

  @impl true
  def handle_cast({:broadcast_all, msg}, state) do
    Enum.each(state, fn {_n, %{socket: s}} -> send_to_socket(s, msg) end)
    {:noreply, state}
  end

  @impl true
  def handle_cast({:force_command, name, command}, state) do
    case Map.fetch(state, name) do
      {:ok, %{socket: sock}} ->
        Logger.info("🎯 force_command to #{name}: #{command}")
        send_to_socket(sock, command)
      :error ->
        Logger.warn("⚠️ force_command failed, unknown client: #{name}")
    end

    {:noreply, state}
  end

  @impl true
  def handle_call(:list, _from, state), do: {:reply, state, state}

  # ────────── Helpers ──────────

  defp set_only_active(registry, name) do
    Enum.into(registry, %{}, fn
      {^name, info} -> {name, %{info | active: true}}
      {other, info} -> {other, %{info | active: false}}
    end)
  end

  defp safe_close(sock) do
    try do
      :gen_tcp.close(sock)
    rescue
      _ -> :ok
    end
  end

  defp send_to_socket(socket, msg) do
    case :gen_tcp.send(socket, msg <> "\n") do
      :ok -> :ok
      {:error, reason} -> Logger.error("❌ send failed to socket #{inspect(socket)}: #{inspect(reason)}")
    end
  end
end
