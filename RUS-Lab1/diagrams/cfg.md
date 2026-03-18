```mermaid

flowchart TD

&#x20;   A\[Main Loop] --> B{Interrupt?}

&#x20;   B -->|Timer| T\[Timer ISR]

&#x20;   B -->|Button| K\[Button ISR]

&#x20;   B -->|Sensor| S\[Sensor ISR]

&#x20;   B -->|UART| U\[UART ISR]

&#x20;   T --> A

&#x20;   K --> A

&#x20;   S --> A

&#x20;   U --> A

