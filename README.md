# esp8266-relay-server

A simple server to accept HTTP requests to control an automatic gate.

## Configure

1. Rename `secrets.example.h` to `secrets.h`
2. Set your secrets

## API

### Headers

All responses include `FirmwareVersion`.

### Endpoints

`GET /`

Health check.

    Status: 204
    Body: 1

`GET /state`

Return gate state. Body returns 0 closed or 1 for open.

    Status: 200
    Body: 1

`GET /open`

Open the gate. Noop if already open.

    Status: 200
    Body: 0

`GET /close`

Close the gate. Noop if already closed.

    Status: 200
    Body: 1

`GET /cycle`

Cycle the gate from the current position.

    Status: 200
    Body: 0

## License

MIT (c) 2018 Nic Haynes
