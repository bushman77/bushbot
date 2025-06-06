defmodule ClientRegistry do
  use GenServer
  require Logger
  defmodule Client do
    @moduledoc """
    Represents a connected client and its character info.
    """

    #@enforce_keys [:socket, :info]
    defstruct socket: nil, info: %{}

    @type t :: %__MODULE__{
      socket: port(),
      info: map()
    }
  end
  # ──────────────────────────────────────────────────────────────
  # Public API
  # ──────────────────────────────────────────────────────────────

  @doc "Start registry"
  def start_link(_), do: GenServer.start_link(__MODULE__, %{}, name: __MODULE__)

  @doc "Get the raw registry state (map of name → %{socket, active})"
  def state(), do: GenServer.call(__MODULE__, :state)

  @doc "Add a client under `name` with `socket` and `active` flag"
  def add(name, socket, obj),    do: GenServer.cast(__MODULE__, {:add, name, socket, obj})

  @doc "Remove a client by name"
  def remove(name),                 do: GenServer.cast(__MODULE__, {:remove, name})

  @doc "Set only this client to active, all others inactive"
  def update_active(name, active),  do: GenServer.cast(__MODULE__, {:update_active, name, active})

  @doc "Broadcast a command to all inactive clients"
  def broadcast_to_inactive(cmd),   do: GenServer.call(__MODULE__, {:broadcast_to_inactive, cmd})

  @doc "Send a single-target command"
  def route_to(target, action),     do: GenServer.cast(__MODULE__, {:route_to, target, action})

  @doc "List the names of all connected clients"
  def clients(), do: state() |> Map.keys()

  @doc "Check if a client with given name is connected"
  def connected?(name), do: Map.has_key?(state(), name)

  @doc "Get the name of the currently active client, or nil if none"
  def active_client() do
    state()
    |> Enum.find(fn {_name, %{active: active}} -> active end)
    |> case do
      {name, _info} -> name
      nil -> nil
    end
  end

  @doc "Get all stored character data"
  def stats(character), do: GenServer.call(__MODULE__, {:stats, character})
  
  # ──────────────────────────────────────────────────────────────
  # GenServer callbacks
  # ──────────────────────────────────────────────────────────────

  @impl true
  def init(state) do
    {:ok, %{}}
  end

  @impl true
  def handle_call(:state, _from, state) do
    {:reply, state, state}
  end

  def handle_call({:stats, character}, _from, state) do
    {:reply, state, state}
  end

  @impl true
  def handle_cast({:add, name, socket, obj}, state) do
    state = Map.put(state, name, %{socket: socket, state: obj})
    state = if obj, do: only_active(name, state), else: state
    {:noreply, state}
  end

  @impl true
  def handle_cast({:remove, name}, state) do
    {:noreply, Map.delete(state, name)}
  end

  @impl true
  def handle_cast({:update_active, name, active}, state) do
    new_state =
      if Map.has_key?(state, name) do
        if active do
          only_active(name, state)
        else
          Map.update!(state, name, fn info -> %{info | active: false} end)
        end
      else
        state
      end

    {:noreply, new_state}
  end

  @impl true
  def handle_cast({:route_to, target, action}, state) do
    case state[target] do
      %{socket: sock} ->
        :gen_tcp.send(sock, action <> "\n")
        Logger.info("📤 Routed to #{target}: #{action}")
      _ ->
        :noop
    end

    {:noreply, state}
  end

  @impl true
  def handle_call({:broadcast_to_inactive, cmd}, _from, state) do
    state
    |> Enum.filter(fn {_name, %{active: a}} -> not a end)
    |> Enum.each(fn {_name, %{socket: sock}} ->
      :gen_tcp.send(sock, cmd <> "\n")
      Logger.debug("📢 Broadcast → #{cmd}")
    end)

    {:reply, :ok, state}
  end

  # ──────────────────────────────────────────────────────────────
  # Helpers
  # ──────────────────────────────────────────────────────────────

  defp only_active(active_name, state) do
    state
    |> Enum.map(fn
      {^active_name, info} -> {active_name, %{info | active: true}}
      {other, info}        -> {other, %{info | active: false}}
    end)
    |> Map.new()
  end
end
