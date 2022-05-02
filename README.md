# Elli-ID-Charger-PV
PV-Überschussladen mit Elli ID Charger - Workaround


The Elli ID Charger from Volkswagen does not support surplus loading actually. This workaround uses the behavior of the overload protection of the Wallbox.

https://global-uploads.webflow.com/5f2271e9820e0bb93e6f3499/5f6a7e1113e3e52cf745fda1_OverloadProtection_Guideline.pdf

Overview of Overload Protection Setup
In order to use the Overload Protection, CT coils need to be installed at the house’s critical
installation point. This might be at the house’s main fuse or any other sub-distribution where the
residential load and the charging power might exceed the local restrictions. These CT coils need to
be wired to the Elli Charger. Since usually the critical installation point and the Elli Charger are
located some distance apart, a data cable in needed to properly establish this wired connection.

This Workaround emulates a load of the house current (sounds funny to me native speakers ahead ;-) :

Assuming we have set a maximum house current of 20A (Imax=20A) and are currently simulating a measured house current of 14A (Isim=14A) for the Elli, because we want it to charge with 6A (Isoll=6A).

The Elli also measures its own power requirement, but it is still 0A (Iwb=0A).
If we then plug in our car, the following happens:
Elli sees the 14A house current and says to the car: "Charge with 6A so that the house current doesn't exceed 20A." No sooner said than done, the car charges with 6A.
Then she determines from her own current measurement that she now needs 6A (Iwb=6A).
Now Elli checks the house electricity again and sees: Oh, it's still 14A, I'm 6A myself, then I can charge more with 6A: "Hey, car, you can charge with 12A".
Then keep going until it's at 16A.
Well, stupid, the Elli charges again with 16A, but should actually only charge with 6A.

So our simulation circuit must not simulate a constant current, but must always simulate the current requirement of the Elli. But for that we need to know the current of the Elli Iwb. 

The Elli requests its own electricity from the meter every 500ms via Modbus RTU (I described this in another thread). So we would have to listen to the Modbus communication with our circuit and record the charging current.

The formula for the simulated current is then:
Isim = Imax - Iset + Iwb

