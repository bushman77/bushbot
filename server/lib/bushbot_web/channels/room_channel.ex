defmodule BushbotWeb.RoomChannel do
  use Phoenix.Channel

  def join("room:lobby", message, socket) do
    IO.inspect(message)
    IO.puts("Client joined room:lobby")
    {:ok, socket}
  end

  def handle_in("ping", _payload, socket) do
    {:reply, {:ok, %{"response" => "pong"}}, socket}
  end

  def handle_in("new_msg", %{"body" => body}, socket) do
    IO.puts("Received message: #{body}")
    broadcast!(socket, "new_msg", %{body: body})
    {:noreply, socket}
  end
end
