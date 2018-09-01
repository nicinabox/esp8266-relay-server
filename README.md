# esp8266-relay-server

A simple server to accept HTTP requests to switch the relay

## Configure

1. Set WIFI_SSID
2. Set WIFI_PASSWORD

## API

`GET /`
```
Status: 200
{ "status": "open" }
```

`GET /open`
```
Status: 204
```
`GET /close`

```
Status: 204
```

## License

MIT (c) 2018 Nic Haynes
