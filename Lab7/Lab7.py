import serial
import time
from serial import EIGHTBITS, PARITY_NONE, STOPBITS_ONE
from xmodem import XMODEM


def connect_to_modem(port, baudrate):
    try:
        # Otwieramy połączenie szeregowe z modemem
        ser = serial.Serial(port, baudrate, bytesize=EIGHTBITS, parity=PARITY_NONE, stopbits=STOPBITS_ONE, timeout=1)
        print(f"Połączono z modemem na porcie {port}")
        return ser
    except serial.SerialException as e:
        print(f"Nie udało się połączyć z modemem: {e}")
        return None

def dial_number(ser, number):
    # Wysyłamy polecenie ATD w celu nawiązania połączenia telefonicznego
    at_command = f"ATD{number}\r"
    ser.write(at_command.encode('utf-8'))
    print(f"Wysłano polecenie ATD: {number}")

    # Oczekiwanie na odpowiedź
    time.sleep(30)
    response = ser.read_all().decode()

    if "CONNECT" in response:
        print("Połączono z modemem!")
        return True
    else:
        print("Nie udało się połączyć.")
        return False


def answer_call(ser):
    print("Oczekiwanie na połączenie przychodzące...")
    try:
        while True:
            if ser.in_waiting > 0:
                response = ser.read_all().decode()
                print(f"Odebrano: {response}")

                # Sprawdzamy, czy modem odbiera sygnał dzwonienia
                if "RING" in response:
                    print("Wykryto połączenie przychodzące.")
                    ser.write("ATA\r".encode())  # Wysyłamy komendę ATA, aby odpowiedzieć
                    time.sleep(5)  # Czekamy na odpowiedź

                    response = ser.read_all().decode()
                    print(f"Odpowiedź po ATA: {response}")

                    if "CONNECT" in response:
                        print("Połączenie zostało nawiązane!")
                        return True
                    else:
                        print("Nie udało się połączyć.")
                        return False
    except KeyboardInterrupt:
        print("Przerwano oczekiwanie na połączenie.")
        return False


def terminal_to_terminal(ser):
    print("Rozpoczynam sesję terminalową. Wpisz 'exit', aby zakończyć.")
    try:
        while True:
            # Odczyt danych
            if ser.in_waiting > 0:
                data = ser.read(ser.in_waiting).decode()
                print(f"Modem: {data}")

            # Wprowadzenie danych od użytkownika
            user_input = input("Ty: ")
            if user_input.lower() == "exit":
                break
            ser.write((user_input + "\r\n").encode(encoding='utf-8'))  # Wysyłamy dane
    except KeyboardInterrupt:
        print("Sesja przerwana.")


def send_file_via_xmodem(ser, filename):
    def getc(size, timeout=1):
        return ser.read(size) or None

    def putc(data, timeout=1):
        return ser.write(data)

    modem = XMODEM(getc, putc)

    # Otwieramy plik do wysłania
    with open(filename, 'rb') as f:
        print(f"Rozpoczynam przesyłanie pliku {filename}...")
        status = modem.send(f)
        if status:
            print("Plik został pomyślnie wysłany!")
        else:
            print("Wystąpił błąd podczas przesyłania pliku.")


# Funkcja główna
def main():
    port = input("Podaj port szeregowy: ")
    baudrate = 9600  # Szybkość transmisji
    modem_number = "3965"  # Numer modemu docelowego

    ser = connect_to_modem('COM1', baudrate)
    if not ser:
        return

    print("Wybierz tryb:")
    print("1: Wykonaj połączenie")
    print("2: Odbierz połączenie")
    choice = input("Twój wybór: ")

    if choice == "1":
        if dial_number(ser, modem_number):
            print("Wybierz tryb:")
            print("1: Konwersacja terminal-terminal")
            print("2: Transmisja pliku (XModem)")
            sub_choice = input("Twój wybór: ")

            if sub_choice == "1":
                terminal_to_terminal(ser)
            elif sub_choice == "2":
                filename = input("Podaj nazwę pliku do wysłania: ")
                send_file_via_xmodem(ser, filename)
            else:
                print("Nieprawidłowy wybór.")
    elif choice == "2":
        if answer_call(ser):
            print("Połączenie zostało nawiązane. Wybierz tryb:")
            print("1: Konwersacja terminal-terminal")
            print("2: Transmisja pliku (XModem)")
            sub_choice = input("Twój wybór: ")

            if sub_choice == "1":
                terminal_to_terminal(ser)
            elif sub_choice == "2":
                filename = input("Podaj nazwę pliku do wysłania: ")
                send_file_via_xmodem(ser, filename)
            else:
                print("Nieprawidłowy wybór.")
        else:
            print("Odbieranie połączenia nie powiodło się.")
    else:
        print("Nieprawidłowy wybór.")

    # Zamykamy połączenie
    ser.close()
    print("Połączenie zostało zakończone.")


if __name__ == "__main__":
    main()


