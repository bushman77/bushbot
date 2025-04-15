# MQ2Elixir

One Paragraph project description goes here

## Getting Started

Quick start instructions to get users up and going

```txt
/plugin MQ2Elixir load
```

### Commands

Describe the commands available and how to use them.

```txt
Give examples
```

### Configuration File

Describe the configuration file and what the settings do

```yaml
- Example goes here
```

## Other Notes

Add additional notes

## Authors

* **Your name** - *Initial work*

See also the list of [contributors](https://github.com/your/project/contributors) who participated in this project.

## Acknowledgments

* Inspiration from...
* I'd like to thank the Thieves' Guild for helping me with all the code I stole...


 ## Introduction

This plugin is designed to enable communication between an EverQuest game client and a custom Elixir server running on the same machine. The main functions of this plugin are: 
starting/stopping the Elixir server, sending commands to it, and reading responses from it. This enables users to extend the functionality of their EQ client with custom scripts 
written in Elixir.

## Dependencies

This plugin requires an implementation of the EverQuest game API which includes functions such as `AddCommand`, `RemoveCommand`, `WriteChatf`, and `DebugSpewAlways`. It also 
assumes that a Windows operating system is being used due to the use of WinAPI functions for handling inter-process communication.

## Usage

To use this plugin, include it in your EQ client's plugin folder and load it using the client's plugin loader. Once loaded, you can send commands to the Elixir server using the 
`/elixir` command followed by your command string. The plugin will automatically start the Elixir server when it is loaded and stop it when it is unloaded.

## Implementation Details

The main functions of this plugin are implemented in the following ways:

- `StartElixir`: This function creates a new process for the Elixir server, sets up anonymous pipes for communication, and starts the server process. It then reads an initial 
message from the server to ensure it has started successfully.
- `StopElixir`: This function terminates the Elixir server process and closes all open handles and threads associated with it.
- `SendToElixir`: This function writes a command string to the output pipe of the Elixir server process, appending a newline character to indicate the end of the command.
- `ReadFromElixir`: This function reads data from the input pipe of the Elixir server process until it encounters a newline character or a timeout occurs. It then cleans the 
response string by removing any trailing newlines and carriage returns.
- `ElixirCommand`: This function is called when the user enters the `/elixir` command in the EQ client. It sends the command string to the Elixir server, reads the response, and 
displays it in the chat window of the EQ client.
- `InitializePlugin`: This function is called by the EQ client's plugin loader when the plugin is loaded. It adds a new command handler for the `/elixir` command and starts the 
Elixir server.
- `ShutdownPlugin`: This function is called by the EQ client's plugin loader when the plugin is unloaded. It removes the command handler for the `/elixir` command and stops the 
Elixir server.

