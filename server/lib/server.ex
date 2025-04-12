defmodule Server do
  use GenServer

  @log_file "mq2elixir_server.log"
  
  ## Client API
  def start_link(_args) do
    GenServer.start_link(__MODULE__, :ok, name: __MODULE__)
  end

  ## Server Callbacks
  @impl true
  def init(:ok) do
    # Initialize logging
    File.write!(@log_file, "", [:write]) # Clear previous log
    log_message("=== Starting MQ2Elixir Server ===")
    
    # Start listener with proper error handling
    case Task.start_link(&listen_loop/0) do
      {:ok, pid} -> {:ok, %{listener_pid: pid}}
      error -> 
        log_message("Failed to start listener: #{inspect(error)}")
        {:stop, :listener_failed}
    end
  end

  ## Command Processing
  defp listen_loop do
    log_message("Listener process started")
    
    # Use :stdio directly with proper flushing
    stdio = IO.stream(:stdio, :line)
    respond("READY")

    Enum.each(stdio, fn line ->
      log_message("Received: #{inspect(line)}")
      response = line |> String.trim() |> process_command()
      log_message("Responding: #{inspect(response)}")
      IO.puts(:stdio, response)
      IO.write(:stdio, "") # Forces flush in newer Elixir versions
    end)
  rescue
    e -> 
      log_message("Listener crashed: #{Exception.format(:error, e, __STACKTRACE__)}")
      raise e
  end

  defp process_command("ping"), do: "pong"
  defp process_command("status"), do: "STATUS: OK"
  defp process_command(""), do: "ERROR: Empty command"
  defp process_command(cmd), do: "ECHO: #{cmd}"

  ## Response Handling (now simplified)
  defp respond(message) do
    IO.puts(:stdio, message)
    IO.write(:stdio, "") # Forces flush
  end

  ## Enhanced Logging
  defp log_message(msg) do
    timestamp = DateTime.utc_now() |> DateTime.to_iso8601()
    log_entry = "[#{timestamp}] #{msg}\n"
    
    File.write!(@log_file, log_entry, [:append, :utf8])
  rescue
    e -> 
      IO.puts(:stderr, "Failed to write log: #{Exception.message(e)}")
  end

  @impl true
  def terminate(reason, _state) do
    log_message("Shutting down. Reason: #{inspect(reason)}")
    :ok
  end
end