## Purchasing guidance

Buy hardware only after downloading the MakerWorld 3MF/STL package and checking its hole diameters, magnet pockets, and fastener locations; printed-model revisions can change the exact sizes. The associated MakerWorld model is published as a "Fully optional smart weather station," so some sensor modules and printed assemblies may be intentionally modular. 

For a permanent outdoor installation, use PETG or ASA for external prints, stainless fasteners, UV-resistant cable ties, and sealed cable entries. PETG/ASA is generally more appropriate than PLA for sun- and weather-exposed outdoor enclosures. 

## Core electronics

| # | Item | Qty. | Required | Notes |
|---:|---|---:|---|---|
| 1 | ESP8266 NodeMCU v3 / ESP-12E development board | 1 | Yes | Main controller; use a board with accessible USB and FLASH button |
| 2 | AHT20 or AHT21 temperature/humidity module | 1 | Yes | I2C address `0x38` |
| 3 | BMP280 barometric-pressure module | 1 | Yes | I2C address typically `0x76` or `0x77`; select a genuine 3.3 V-compatible module |
| 4 | ENS160 air-quality module | 1 | Optional | Provides TVOC, eCO2 and AQI; I2C address `0x52` or `0x53` |
| 5 | AS5600 magnetic rotary encoder module | 1 | Yes | Wind-vane direction sensing; I2C address `0x36` |
| 6 | BH1750 light/lux module | 1 | Optional | I2C address `0x23`; intended for use behind the PETG light cover |
| 7 | A3144 Hall-effect sensor, bare sensor or breakout | 2 | Yes | One for rain gauge and one for anemometer |
| 8 | 5 V DC power supply, 1 A minimum | 1 | Yes | USB wall adaptor or enclosed 5 V supply; choose an outdoor-safe arrangement |
| 9 | USB cable or 5 V lead for NodeMCU | 1 | Yes | Depends on whether the enclosure uses USB or Vin/GND wiring |
| 10 | I2C wiring harness / 4-core cable | 1–3 m | Yes | 3.3 V, GND, SDA, SCL between control enclosure and I2C modules |
| 11 | 3-core sensor cable | 3–6 m | Yes | For Hall sensors; use stranded, weather-resistant cable |
| 12 | Small terminal block, WAGO-style connectors, or solder board | 1 | Recommended | Simplifies field-serviceable power and sensor connections |
| 13 | 100 nF ceramic capacitors | 2–4 | Recommended | Place near remote Hall sensors if long cable runs cause false triggers |
| 14 | 10 kOhm resistors | 2 | Optional | External pull-ups only if the selected Hall-sensor board does not provide them |

The documented wiring uses I2C on GPIO 4/D2 and GPIO 5/D1, rain pulses on GPIO 14/D5, and wind pulses on GPIO 12/D6. The firmware uses internal pull-ups for the pulse inputs, but good external wiring and optional local decoupling can improve reliability on long outdoor cables. 

## Mechanical hardware

| # | Item | Qty. | Required | Notes |
|---:|---|---:|---|---|
| 15 | 608Z or 608ZZ ball bearings | 2 | Yes | One for the anemometer and one for the wind vane |
| 16 | Galvanized steel pipe, 3/4 inch | 1 | Yes | Length to suit mounting location; project mount is designed for this diameter |
| 17 | M8 stainless bolt | 2–4 | Yes | Pole-clamp bolts; choose length from final clamp geometry, commonly 25–40 mm |
| 18 | M8 stainless washer | 4–8 | Yes | Use under bolt heads and/or nuts as required |
| 19 | M8 stainless nut or nyloc nut | 2–4 | Depends on model | Needed if the printed part does not use threaded M8 holes directly |
| 20 | M3 stainless machine screws | 10–20 | Recommended | Sensor boards, electronics enclosure, covers and cable clamps |
| 21 | M3 washers | 10–20 | Recommended | Prevents screw heads damaging printed parts |
| 22 | M3 heat-set inserts or M3 nuts | 10–20 | Recommended | Use whichever retention method the selected print files support |
| 23 | M2/M2.5 screws and nuts | 4–12 | Optional | Some NodeMCU and small sensor PCBs use these rather than M3 |
| 24 | Stainless steel rod / shaft | 2 | Depends on model | For vane and anemometer axles if shafts are not printed |
| 25 | Shaft retaining collars, E-clips, or lock nuts | 2–4 | Recommended | Prevents the rotating assemblies lifting off in gusts |
| 26 | Small counterweight for vane | 1 | Yes | Inserted in the vane tip to stabilise response, as specified by the project |
| 27 | Small hinges/pivot pins/axle for tipping bucket | 1 set | Yes | Exact item depends on the rain-gauge STL design |
| 28 | Small calibration screws for bucket adjustment | 2 | Yes | One adjustment point per bucket side, where the print supports it |

The existing project documentation explicitly calls for two 608Z bearings, a 3/4-inch galvanized pipe, an internal vane counterweight, and M8 hardware for securing the printed pole clamp. It recommends tapping the printed M8 threads before installing clamp bolts. 

## Magnets and motion parts

| # | Item | Qty. | Required | Notes |
|---:|---|---:|---|---|
| 29 | Neodymium magnet for AS5600 encoder | 1 | Yes | Diametric magnet strongly preferred; size must match the AS5600 holder/pocket |
| 30 | Neodymium magnets for rain bucket | 2 | Yes | The repository specifies two magnets in the modified rain bucket |
| 31 | Neodymium magnet(s) for anemometer | 1–2 | Yes | One per revolution by default; use more only if firmware configuration is updated |
| 32 | Two-part epoxy or cyanoacrylate | 1 | Recommended | Retains magnets; epoxy is generally more impact- and weather-resistant |
| 33 | Non-magnetic fasteners near AS5600 | As needed | Recommended | Nylon or stainless hardware helps avoid disrupting the encoder’s magnetic field |
| 34 | Light machine oil / sewing-machine oil / gun oil | 1 | Yes | Very small quantity after bearing cleaning |
| 35 | Isopropyl alcohol, ideally 90%+ | 100–250 ml | Yes | Two 15-minute bearing-cleaning baths are specified |

The rain-gauge description specifically states that two magnets are inserted in the tipping bucket. The anemometer defaults to one magnet/pulse per revolution, while the AS5600 requires a separate rotating magnet for contactless vane direction sensing. 

**Important:** do not substitute a random axial disc magnet for the AS5600 without testing. The AS5600 is normally designed for a suitably centred **diametrically magnetised** magnet. Confirm smooth angle readings through a full rotation before sealing the vane enclosure.

## Printed components

| # | Printed assembly | Qty. | Material | Notes |
|---:|---|---:|---|---|
| 36 | Main pole-clamp base | 1 | PETG or ASA | Supports all assemblies on the pipe |
| 37 | Sensor support arms | 1 set | PETG or ASA | Press-fit into the main mount according to repository notes |
| 38 | Electronics enclosure and lid | 1 set | PETG or ASA | Protects NodeMCU, wiring and connectors |
| 39 | Radiation-shield plates/louvres | 1 set | White PETG or ASA | Holds temperature/humidity/pressure sensors in moving air |
| 40 | Environmental sensor carrier | 1 | PETG or ASA | Internal mount for AHT20 and BMP280 |
| 41 | ENS160 ventilated holder/enclosure | 1 | PETG or ASA | Optional if ENS160 is installed |
| 42 | BH1750 cover, holder and light-filter enclosure | 1 set | PETG or ASA | Keep the transparent cover clear and clean |
| 43 | Anemometer body, hub and cups | 1 set | PETG or ASA | Must be balanced and low-friction |
| 44 | Wind-vane body, fin, hub and AS5600 case | 1 set | PETG or ASA | Includes space for counterweight and encoder magnet |
| 45 | Rain funnel and tipping-bucket assembly | 1 set | PETG or ASA | Use durable material because water, UV and movement are continuous |
| 46 | Covers, caps, cable clips and strain-relief parts | As supplied | PETG or ASA | Print spares for likely service items |

The project’s MakerWorld models include enclosures, radiation shields, sensor arms, and mounting brackets. Its mechanical notes describe press-fitting the support arms into the pole base and using a rubber mallet if necessary. 

## Wiring and weatherproofing

| # | Item | Qty. | Required | Notes |
|---:|---|---:|---|---|
| 47 | IP65/IP67 cable glands | 4–8 | Yes | Select for actual cable diameters |
| 48 | Heat-shrink tubing, preferably adhesive-lined | 1 kit | Yes | Waterproofs solder joints and adds strain relief |
| 49 | Ferrules or crimp terminals | 1 kit | Recommended | Useful for terminal blocks and reliable service work |
| 50 | UV-resistant cable ties | 1 pack | Yes | Use black outdoor-rated ties, not indoor white nylon ties |
| 51 | Neutral-cure silicone sealant | 1 tube | Recommended | Seal non-serviceable cable penetrations and external seams |
| 52 | Foam gasket tape or enclosure gasket | 1 roll | Recommended | Improves lid seal while retaining serviceability |
| 53 | Dielectric grease | 1 small tube | Optional | Helps protect low-voltage connectors from moisture |
| 54 | Cable conduit, braided sleeve, or spiral wrap | As needed | Recommended | Protects exposed cable runs from UV, animals and abrasion |
| 55 | Label tape or heat-shrink wire labels | 1 set | Recommended | Label `5V`, `GND`, `SDA`, `SCL`, `RAIN`, and `WIND` before enclosure closure |

Keep all cable entries facing downward where possible and create a drip loop below each enclosure entry. That protects the electronics more effectively than sealant alone.

## Tools and consumables

| # | Tool or consumable | Qty. | Required | Purpose |
|---:|---|---:|---|---|
| 56 | Soldering iron and solder | 1 | Usually | Sensor wiring and robust outdoor joints |
| 57 | Wire stripper and side cutters | 1 | Yes | Cable preparation |
| 58 | Multimeter | 1 | Yes | Verify voltage, continuity and sensor wiring |
| 59 | M8 thread tap and handle | 1 | Yes | Cleans printed pole-mount threads as advised by the project |
| 60 | Small screwdriver set / hex keys | 1 set | Yes | Sensor and enclosure fasteners |
| 61 | Rubber mallet | 1 | Recommended | Gentle assembly of press-fit sensor arms |
| 62 | Bubble level | 1 | Yes | Rain-gauge levelling |
| 63 | Syringe or graduated measuring jug | 1 | Yes | Rain-gauge calibration using repeatable water volume |
| 64 | Compass or phone compass | 1 | Yes | Physical north alignment before AS5600 calibration |
| 65 | Threadlocker, medium strength | 1 | Recommended | Use sparingly on metal-to-metal rotating-assembly fasteners |
| 66 | Small file, deburring tool and drill bits | 1 set | Recommended | Clears printed holes, bearing pockets and cable pass-throughs |

The project’s rain calibration procedure uses 6 ml of water per bucket side, and its north calibration requires physically pointing the vane north before storing the direction offset. 

## Recommended spare parts

Keep these on hand because they are small, inexpensive, and most likely to fail or be lost during maintenance:

- One spare A3144 Hall sensor.
- Two spare 608Z/608ZZ bearings.
- One spare AS5600 magnet.
- Four spare rain/anemometer magnets.
- A spare NodeMCU.
- Extra M3 screws, M3 inserts/nuts, and M8 clamp hardware.
- One replacement BH1750 transparent cover or a small piece of suitable clear PETG.
- Spare cable glands and UV-resistant cable ties.
- A small bottle of light bearing oil.

## Repository-ready BOM note

> **Quantities are for one complete, fully equipped weather station.** Optional components include the ENS160 air-quality sensor and BH1750 light sensor. Before ordering magnets, shafts, and screws, check the dimensions in the downloaded 3MF/STL files, as those printed models define the exact pocket diameter, shaft fit, and fastener length. The firmware must be configured to match the final anemometer magnet count, arm radius, rain-gauge calibration, and AS5600 north offset. 