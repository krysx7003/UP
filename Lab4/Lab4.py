import tkinter
import serial
import serial.tools.list_ports
import tkintermapview


def gga_nmea_to_coordinates(nmea_string):
    # Split the NMEA string by commas
    parts = nmea_string.split(',')

    # Check if the string is a GGA sentence
    if parts[0] != '$GPGGA':
        raise ValueError("Invalid GGA sentence")

    # Extract latitude and longitude
    lat_str = parts[2]
    lat_dir = parts[3]
    lon_str = parts[4]
    lon_dir = parts[5]

    # Convert latitude
    lat_degrees = int(lat_str[:2])  # First two characters are degrees
    lat_minutes = float(lat_str[2:])  # Remaining characters are minutes
    latitude = lat_degrees + (lat_minutes / 60)
    if lat_dir == 'S':
        latitude = -latitude  # South is negative

    # Convert longitude
    lon_degrees = int(lon_str[:3])  # First three characters are degrees
    lon_minutes = float(lon_str[3:])  # Remaining characters are minutes
    longitude = lon_degrees + (lon_minutes / 60)
    if lon_dir == 'W':
        longitude = -longitude  # West is negative

    return latitude, longitude, float(parts[9]), float(parts[11])


def parse_gga_time(sentence):
    """Parsuje zdanie NMEA typu $GPGGA i zwraca czas UTC w formacie HH:MM:SS."""
    if sentence.startswith("$GPGGA"):
        parts = sentence.split(',')
        if len(parts) >= 2 and parts[1]:  # Drugie pole to czas
            utc_time = parts[1]
            # Podział na godziny, minuty i sekundy
            hours = int(utc_time[:2]) + 1
            minutes = utc_time[2:4]
            seconds = utc_time[4:6]
            return f"{hours}:{minutes}:{seconds} UTC+1"
    return None


def initial_verification(raw_sentence):
    try:
        if raw_sentence[0] != '$' or raw_sentence[-1] != '\n':
            raise Exception("wrong string")
        xor = 0
        for val in raw_sentence[1:raw_sentence.find('*') - 1].split(','):
            for char in val:
                xor = xor ^ ord(char)
        xor2 = 0
        mult = 16
        for char in raw_sentence[raw_sentence.find('*') + 1: raw_sentence.find('\r')]:
            xor2 += int(char) * mult
            mult /= 16
        return xor == xor2

    except Exception as e:
        return False


def parse_rmc_date(sentence):
    """Parsuje zdanie NMEA typu $GPRMC i zwraca datę w formacie DD-MM-YYYY."""
    if sentence.startswith("$GPRMC"):
        parts = sentence.split(',')
        if len(parts) >= 10 and parts[9]:  # Dziesiąte pole to data
            rmc_date = parts[9]
            # Podział na dzień, miesiąc i rok
            day = rmc_date[:2]
            month = rmc_date[2:4]
            year = rmc_date[4:6]
            # Zakładamy, że lata zaczynają się od 2000 roku
            full_year = f"20{year}" if int(year) < 50 else f"19{year}"
            return f"{day}-{month}-{full_year}"
    return None


def map_init(com_port):
    root_tk = tkinter.Tk()
    root_tk.geometry(f"{800}x{600}")
    root_tk.title("map_view_example.py")

    map_widget = tkintermapview.TkinterMapView(root_tk, width=800, height=600, corner_radius=0)
    map_widget.place(relx=0.5, rely=0.5, anchor=tkinter.CENTER)
    map_widget.set_position(51.109230, 17.060283)  # Wrocław, Poland
    map_widget.set_zoom(10)

    # Add labels for latitude, longitude, and altitude
    lat_label = tkinter.Label(root_tk, text="Szerokość: ")
    lat_label.place(relx=0.1, rely=0.9, anchor=tkinter.W)
    lon_label = tkinter.Label(root_tk, text="Długość: ")
    lon_label.place(relx=0.4, rely=0.9, anchor=tkinter.W)
    alt_label = tkinter.Label(root_tk, text="Wysokość: ")
    alt_label.place(relx=0.7, rely=0.9, anchor=tkinter.W)

    root_tk.after(200, gps_main_loop, map_widget, com_port, lat_label, lon_label,
                  alt_label)  # Schedule gps_main_loop to run after 200 ms
    root_tk.mainloop()
    return map_widget


def gps_main_loop(map_widget, com_port, lat_label, lon_label, alt_label):
    marker = map_widget.set_position(51.109230, 17.060283, "Lokalizacja", marker=True)

    def update_position():
        raw_data = com_port.readline().decode('ascii', errors='replace')
        if not initial_verification(raw_data):
            map_widget.after(200, update_position)
            return

        raw_data = raw_data.strip()
        print(f"\nOtrzymane zdanie NMEA: {raw_data}")
        rmc_date = parse_rmc_date(raw_data)
        if rmc_date:
            print(f"Data RMC: {rmc_date}")

        time_utc = parse_gga_time(raw_data)
        print(f"Czas UTC: {time_utc}")

        latitude, longitude, height, geoid_height = gga_nmea_to_coordinates(raw_data)
        print(
            f"Szerokość geograficzna: {latitude} N, długość geograficzna: {longitude} E\n Wysokość nad poziomem morza : {height} \t Wysokość Geoidy : {geoid_height}")
        marker.set_position(latitude, longitude)
        map_widget.set_position(latitude, longitude)
        # Update labels with the new position and altitude
        lat_label.config(text=f"Szerokość: {latitude}")
        lon_label.config(text=f"Długość: {longitude}")
        alt_label.config(text=f"Wysokość: {height}")

        map_widget.after(200, update_position)  # Schedule the next update

    update_position()  # Start the first update


def main():
    ports = serial.tools.list_ports.comports()
    for port, desc, hwid in sorted(ports):
        print(f"{port}: {desc}")
      
    i = int(input("Wybiez port: \n>"))-1
    port = ports[i].device
    try:
        com_port = serial.Serial(port, baudrate=9600, timeout=1)
        print(f"Nasłuchiwanie na porcie {port}...")
        map_init(com_port)
    except serial.SerialException as e:
        print(f"Błąd: Nie można otworzyć portu szeregowego - {e}")
        return
if __name__ == "__main__":
    main()
