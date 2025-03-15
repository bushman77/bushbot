defmodule BushbotWeb.UserSocket do
  use Phoenix.Socket

  ## Channels
  channel "room:lobby", BushbotWeb.RoomChannel

  def connect(_params, socket, _connect_info) do
    {:ok, socket}
  end

  def id(_socket), do: nil

  ## Specify the V2 JSON serializer
  transport(:websocket, Phoenix.Transports.WebSocket,
    serializer: [Phoenix.Socket.V2.JSONSerializer]
  )
end
