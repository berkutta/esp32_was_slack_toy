# MQTT (/Slack) controlled Servo

Litte shitty software to hook up a Servo via ESP32 to MQTT.

## Protocol description

Control commands are done in /gadget/rat/control, acknowledges in /gadget/rat/feedback

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
