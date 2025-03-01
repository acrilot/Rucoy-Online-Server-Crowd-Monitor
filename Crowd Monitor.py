import subprocess
import sys

def check_and_install_dependencies(dependencies):
    """
    Checks and installs required dependencies.
    :param dependencies: A list of dependencies to verify and install
    """
    for dependency in dependencies:
        try:
            __import__(dependency)
        except ImportError:
            print(f"The module '{dependency}' is not installed. Installing...")
            subprocess.check_call([sys.executable, "-m", "pip", "install", dependency])
            print(f"The module '{dependency}' was successfully installed.")

if __name__ == "__main__":
    # List your required dependencies here
    required_dependencies = ["requests", "time", "datetime"]  

    check_and_install_dependencies(required_dependencies)

import requests
import time
from datetime import datetime

print("""
This application orders Rucoy Online servers by online characters,
refreshes ordered list simultaneously with data source and prints
latest update time of list.

Data source is 'https://www.rucoyonline.com/server_list.json'
If it stops refreshing list, rerun program.

Made by acrilot.
""")

top_n_value = 24

def fetch_server_data():
    url = "https://www.rucoyonline.com/server_list.json"
    try:
        response = requests.get(url)
        if response.status_code == 200:
            data = response.json()
            return data.get('servers')
        else:
            print(f"Error fetching data: {response.status_code}")
            return None
    except Exception as e:
        print(f"Error occurred: {e}")
        return None

def display_top_servers(servers, top_n=5):
    if isinstance(servers, list):
        sorted_servers = sorted(servers, key=lambda x: x['characters_online'], reverse=True)
        
        top_servers = sorted_servers[:top_n]
        
        server_list = "\n".join([f"{idx + 1}) {server['name']} - {server['characters_online']} characters online" 
                                for idx, server in enumerate(top_servers)])
        
        current_time = datetime.now().strftime("%d/%m/%Y - %H:%M:%S")

        return f"Server List (Ordered by characters online):\n\n" \
               f"{server_list}\n\n" \
               f"(Latest Update {current_time})\n\n"
    else:
        return "Unexpected data format, 'servers' should be a list."

alert_sent = {}

def main():
    print(display_top_servers(fetch_server_data(), top_n=top_n_value))
    previous_characters_online = {}
    message_sent = False

    while True:
        servers = fetch_server_data()
        if servers:
            current_time = time.time()
            
            change_detected = False
            for server in servers:
                server_name = server['name']
                current_characters_online = server['characters_online']

                if server_name in previous_characters_online:
                    if current_characters_online != previous_characters_online[server_name]:
                        change_detected = True

                previous_characters_online[server_name] = current_characters_online

            if change_detected and not message_sent:
                server_list = display_top_servers(servers, top_n=top_n_value)
                print(server_list)
                message_sent = True
            elif not change_detected:
                message_sent = False

        else:
            print("Failed to retrieve server data.")
        
        time.sleep(1)


if __name__ == "__main__":
    main()
