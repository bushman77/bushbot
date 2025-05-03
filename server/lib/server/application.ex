defmodule ElixirTcpServer.Application do
  use Application

  def start(_type, _args) do
    children = [
      {Task.Supervisor, name: ElixirTcpServer.TaskSupervisor},
      ElixirTcpServer.ClientRegistry,
      ElixirTcpServer.TCP,
      ElixirTcpServer.SpellLoader
    ]

    opts = [strategy: :one_for_one, name: ElixirTcpServer.Supervisor]
    Supervisor.start_link(children, opts)
  end
end
