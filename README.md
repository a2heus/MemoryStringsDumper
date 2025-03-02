# MemoryStringsDumper

MemoryStringsDumper is a Windows console application written in C++ that allows you to:

- Display a filtered list of user processes (excluding system and idle processes) in a paginated view (10 per page).
- Navigate the list using the arrow keys:
  - **Up/Down**: Move the selection within the current page.
  - **Left/Right**: Change the page.
  - **Enter**: Confirm the process selection.
- Dump the memory of the selected process.
- Extract printable strings (minimum 4 characters long) from the memory dump.
- 
## Requirements

- Windows operating system.
- C++11 (or later) compiler.
- Administrator privileges (recommended) to access process memory.

## Build Instructions

1. Clone the repository:
   ```bash
   git clone https://github.com/<your-username>/MemoryDumpSelector.git
   ```
2. Open the project in your preferred C++ IDE (e.g., Visual Studio) or compile via the command line.
3. Make sure your compiler supports C++11 or later.
4. Build the project.

## Usage

1. Run the compiled executable.  
2. Navigate the process list using:
   - **Up/Down arrows** to move the selection within the current page.
   - **Left/Right arrows** to change the page.
   - Press **Enter** to select the process.
3. The program will attempt to open the selected process, dump its memory, and extract printable strings.
4. The extracted strings will be saved in a file with a timestamped name (e.g., `20230315123045.txt`).

## Disclaimer

- This application is for educational and testing purposes only.
- Dumping and reading the memory of processes may be subject to legal restrictions. Use responsibly and only on processes for which you have permission.
- Running the application may require administrator privileges.

## License

This project is licensed under the MIT License.

## Made by hash

Enjoy!
