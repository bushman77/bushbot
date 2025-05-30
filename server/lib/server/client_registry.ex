defmodule Character do
  @moduledoc """
  Keeps one entry per connected client:

      %{
        name: "Sandayar",
        port: #Port<0.11>,
        character: %{ ...raw JSON map from C++... }
      }

  The Agent’s state is a *map keyed by socket port* so look-ups are O(1),
  but `all/0` returns just the **values**, giving you the list you asked for.
  """

  use Agent

  ## ── lifecycle ──────────────────────────────────────────────
  def start_link(_opts) do
    Agent.start_link(fn -> %{} end, name: __MODULE__)
  end

  ## ── public API ─────────────────────────────────────────────

  @doc "Insert a placeholder as soon as the TCP connection is accepted."
  def add(socket) do
    Agent.update(__MODULE__, fn state ->
      Map.put_new(state, socket, %{port: socket})
    end)
  end

  @doc """
  Store the *full* character info after we receive the JSON
  from the client.
  """
  def set_character_info(socket, %{"name" => name} = info) do
    Agent.update(__MODULE__, fn state ->
      Map.put(state, socket, %{name: name, port: socket, character: info})
    end)
  end

  @doc "Remove client on disconnect."
  def remove(socket) do
    Agent.update(__MODULE__, &Map.delete(&1, socket))
  end

  @doc """
  Return **list of maps** (not the internal socket→map structure).
  Exactly what you wanted for `Character.all/0`.
  """
  def all do
    Agent.get(__MODULE__, &Map.values/1)
  end

  @doc "Look a client up by name (handy for /cmd routing)."
  def get_by_name(name) do
    Agent.get(__MODULE__, fn state ->
      Enum.find_value(state, fn {_socket, data} ->
        if data[:name] == name, do: data
      end)
    end)
  end
end
