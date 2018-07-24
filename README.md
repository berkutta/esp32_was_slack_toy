# MQTT (/Slack) controlled Servo

Little shitty software to hook up a Servo via ESP32 to MQTT.

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

### Get Software version

```
{  
   "mode":"version"
}
```

### Update Software

The Update feature is very experimental and should just be used if you have access to the hardware in case something goes wrong!

```
{  
   "mode":"ota",
   "server":"domain.ch",
   "filename":"file.bin"
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
