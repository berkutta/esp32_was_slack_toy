# MQTT (/Slack) controlled Servo

Litte shitty software to hook up a Servo via ESP32 to MQTT.

## Protocol description

Everything is done in topic /gadgets/rat

### Let Servo do it's thing

```
{
   "mode":"run",
   "minimum":50,
   "maximum":150,
   "amount":1,
   "delay":200
}
```

## Protocol Acks

### OK
```
{  
   "status":"OK"
}
```

### Not Ok
```
{  
   "status":"Error"
}
```
