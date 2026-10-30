# Peripheral Detector and Keyboard Monitoring System

## 1. Introduction

This project explores Windows system programming using the C language and native Windows APIs. It consists of two main components: a **Peripheral Detector**, designed to enumerate hardware devices recognized by Windows, and a **Keyboard Monitoring Demonstration**, which explores keyboard events and client-server communication.

The main objective is to understand how applications interact with the operating system, access hardware information, process system events, and communicate through a network.

C was selected because it provides low-level control, direct access to Windows APIs, and a practical way to understand structures, pointers, callbacks, and memory management.

## 2. Peripheral Detector

### 2.1 Project description

The Peripheral Detector identifies and displays hardware devices recognized by Windows. Depending on the device classes and interfaces requested, it can enumerate peripherals such as USB devices, PCI devices, and ACPI-related devices.

The program retrieves device information and displays relevant properties, such as device names, descriptions, and hardware identifiers when available.

### 2.2 Why SetupAPI?

The project uses **Windows SetupAPI**, a native Windows API designed to manage device installation and retrieve device information.

SetupAPI provides functions that allow the application to:

- Retrieve device information sets.
- Enumerate device entries.
- Access device properties.
- Identify hardware through device identifiers.

We selected SetupAPI because it is specifically designed for Windows device enumeration and provides structured access to information managed by the operating system.

Other technologies were considered:

- **WMI:** Provides hardware information through a higher-level management interface, but commonly requires COM programming and WQL queries.
- **Windows Registry:** Contains hardware-related information, but direct registry inspection is less suitable for general device enumeration and can depend on implementation details.
- **DeviceIoControl:** Useful for sending specific requests to device drivers, but unnecessary for basic device discovery.

SetupAPI was therefore the most appropriate choice for this project because it offers documented device-enumeration functionality directly from C.

## 3. Keyboard Monitoring Demonstration

### 3.1 What is a Windows Hook?

A Windows Hook is a mechanism that allows an application to receive certain system events through a callback function.

For keyboard-event demonstrations, Windows provides the `WH_KEYBOARD_LL` hook type, used with the `SetWindowsHookEx` function.

When a relevant keyboard event occurs, Windows invokes the registered callback function and provides information about that event.

This event-driven mechanism is useful for understanding how Windows processes keyboard events without continuously polling the keyboard.

### 3.2 Why use a Hook?

A hook was selected because it demonstrates how applications can interact with operating-system events through callbacks.

Compared with polling, which repeatedly checks keyboard state, hooks provide an event-driven approach.

Other alternatives include:

- **Polling:** Repeatedly checks keyboard state and can consume unnecessary CPU resources.
- **Raw Input API:** Provides device-oriented input information and is useful when an application needs to distinguish input devices.
- **Window messages:** Suitable when keyboard input is only required inside the application's own window.

Hooks are useful for studying Windows event handling, but their broader scope requires particular attention to transparency, user consent, and privacy.

The demonstration should operate visibly and use harmless test input rather than recording passwords or private information.

### 3.3 Keyboard event processing

Windows keyboard events provide information such as virtual-key codes and scan codes.

These values do not necessarily represent the final character typed. The resulting character can depend on keyboard layout and modifier keys such as Shift.

Windows provides functions such as `ToUnicode` to help translate keyboard information into characters using the current keyboard state.

## 4. Sender and Receiver Communication

### 4.1 Why Winsock2?

The sender and receiver component explores network communication using **Winsock2**, the Windows Sockets API.

Winsock2 provides the functions required to create sockets and exchange information between applications over a network.

Important functions include:

- `WSAStartup()`: Initializes Winsock.
- `socket()`: Creates a socket.
- `bind()`: Associates a socket with a local address and port.
- `listen()`: Waits for incoming connections.
- `accept()`: Accepts a connection.
- `connect()`: Establishes a client connection.
- `send()` and `recv()`: Exchange data.
- `closesocket()`: Closes a socket.
- `WSACleanup()`: Releases Winsock resources.

### 4.2 Why TCP instead of UDP?

TCP was selected for the communication demonstration because it provides reliable, ordered delivery through a connection-oriented byte stream.

UDP is connectionless and does not provide the same delivery guarantees.

For a basic sender-receiver project, TCP makes it easier to understand connections, communication, and data exchange.

However, TCP does not preserve message boundaries. Applications must define their own message framing because one `send()` call does not necessarily correspond to one `recv()` call.

The communication demonstration should exchange explicit, non-sensitive test messages between authorized systems.

## 5. Conclusion

This project demonstrates several important Windows programming concepts using C.

The Peripheral Detector uses SetupAPI to retrieve hardware information. The keyboard-event demonstration introduces Windows hooks and callback-based event processing. Finally, Winsock2 introduces network programming and TCP communication between applications.

Through this project, we gained practical experience with:

- Windows API programming.
- Device enumeration and hardware identification.
- Structures, pointers, and callbacks.
- Windows event handling.
- TCP client-server communication.
- Error handling and resource management.

Overall, the project provides an introduction to Windows internals and systems programming while emphasizing responsible use of operating-system capabilities and respect for user privacy.