defmodule Server do
  use GenServer

  ## Client API
  def start_link(_args) do
    GenServer.start_link(__MODULE__, :ok, name: __MODULE__)
  end

  ## Server Callbacks
  @impl true
  def init(:ok) do
    respond("=== Starting MQ2Elixir Server ===")

    case Task.start_link(&listen_loop/0) do
      {:ok, pid} -> {:ok, %{listener_pid: pid}}
      error ->
        respond("Failed to start listener: #{inspect(error)}")
        {:stop, :listener_failed}
    end
  end

  ## Command Processing
  defp listen_loop do
    respond("READY")

    stdio = IO.stream(:stdio, :line)

    Enum.each(stdio, fn line ->
      line
      |> String.trim()
      |> process_command()
      |> respond()
    end)
  rescue
    e ->
      respond("Listener crashed: #{Exception.format(:error, e, __STACKTRACE__)}")
      raise e
  end

  defp process_command("ping"), do: "pong"
  defp process_command("status"), do: "STATUS: OK"
  defp process_command(""), do: "ERROR: Empty command"
  defp process_command(cmd), do: "ECHO: #{cmd}"

  ## Pipe Response Writer
  defp respond(message) do
    IO.puts(:stdio, message)
    IO.write(:stdio, "")  # Forces flush in older versions
  end

  @impl true
  def terminate(reason, _state) do
    respond("Shutting down. Reason: #{inspect(reason)}")
    :ok
  end
end
