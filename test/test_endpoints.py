import requests
import json
import time

# Configuration
ESP32_IP = "192.168.0.100"  # Replace with the actual IP address of your ESP32
BASE_URL = f"http://{ESP32_IP}"

def test_root_endpoint():
    print("Testing root endpoint...")
    url = f"{BASE_URL}/"
    response = requests.get(url)
    assert response.status_code == 200
    print("Root endpoint passed!")
    print(response.text)

def test_data_endpoint():
    print("Testing data endpoint...")
    url = f"{BASE_URL}/data"
    response = requests.get(url)
    assert response.status_code == 200
    json_response = response.json()
    assert isinstance(json_response, list)
    print("Data endpoint passed!")
    print(json.dumps(json_response, indent=4))

def test_data_reset_endpoint():
    print("Testing data&reset endpoint...")
    url = f"{BASE_URL}/data&reset"
    response = requests.get(url)
    assert response.status_code == 200
    json_response = response.json()
    assert isinstance(json_response, list)
    print("Data&reset endpoint passed!")
    print(json.dumps(json_response, indent=4))
    time.sleep(10)  # Give some time for the ESP32 to reset

def test_monitor_endpoint():
    print("Testing monitor endpoint...")
    url = f"{BASE_URL}/monitor"
    response = requests.get(url)
    assert response.status_code == 200
    assert "<table>" in response.text
    print("Monitor endpoint passed!")
    print(response.text)

def run_tests():
    test_root_endpoint()
    test_monitor_endpoint()
    test_data_endpoint()
    test_data_reset_endpoint()
    print("All tests passed!")

if __name__ == "__main__":
    run_tests()
