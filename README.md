# esp8266-relay-server

A simple server to accept HTTP requests to control an automatic gate.

## Configure

1. Set WIFI_SSID
2. Set WIFI_PASSWORD

## API

`GET /`

Return gate status. Body returns 0 or 1 for closed and open, respectively.

    Status: 200
    => 1

`GET /open`

Open the gate. Noop if already open.

    Status: 204

`GET /close`

Close the gate. Noop if already closed.

    Status: 204

`GET /cycle`

Cycle the gate from the current position.

    Status: 204

## License

MIT (c) 2018 Nic Haynes
