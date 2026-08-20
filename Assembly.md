## Table of Contents

<!-- TOC -->

- [Table of Contents](#table-of-contents)
- [Intro](#intro)
- [Before you start](#before-you-start)
- [Print preparation](#print-preparation)
- [Main pole mount](#main-pole-mount)
- [Electronics enclosure](#electronics-enclosure)
- [Temperature, humidity, pressure, air-quality and light arm](#temperature-humidity-pressure-air-quality-and-light-arm)
- [Radiation shield orientation](#radiation-shield-orientation)
- [Wind vane](#wind-vane)
- [Anemometer](#anemometer)
- [Rain gauge](#rain-gauge)
    - [Bucket calibration](#bucket-calibration)
- [Final outdoor layout](#final-outdoor-layout)
- [Commissioning checklist](#commissioning-checklist)
- [Final note](#final-note)

<!-- /TOC -->

## Intro

Below is a practical **draft assembly chapter** you can add to the repository README. It fills the mechanical gap while staying aligned with the documented electronics, pinout, calibration options, and MakerWorld model set. The project's printable parts include enclosures, radiation shields, sensor arms, and mounting brackets; the firmware expects an ESP8266 NodeMCU with the listed I2C and pulse sensors.
## Before you start

Print outdoor parts in **PETG, ASA, or another UV- and weather-resistant material**. Avoid PLA for permanent outdoor use, especially for the radiation shield, moving parts, and pole mount, because heat and UV exposure can cause deformation over time.

Recommended preparation:

- Print one small fit-test section first for bearing seats, magnet pockets, threaded holes, and the NodeMCU enclosure.
- Clean support material, stringing, and elephant-foot buildup from all mating surfaces.
- Dry-fit every bearing, shaft, cover, and arm before applying glue or installing electronics.
- Use stainless-steel fasteners where possible; ordinary steel hardware will rust outdoors.
- Keep the final station level and mount it clear of roofs, walls, trees, exhaust outlets, and local heat sources.

## Print preparation

The MakerWorld download is the source of truth for the exact names and shapes of printable components, so use the object names from its 3MF/STL package where they differ from the generic labels below.

| Assembly area | Typical printed parts | Recommended settings |
|---|---|---|
| Main mount | Pole clamp/base, arm sockets, caps | PETG/ASA, 4 walls, 30–40% infill |
| Sensor housing | Electronics box, lid, cable glands/covers | PETG/ASA, 3–4 walls, 20–30% infill |
| Radiation shield | Plates/louvres, spacers, sensor carrier | White PETG/ASA, 3 walls, 20% infill |
| Anemometer | Rotor, cups, hub, bearing holders | PETG/ASA, 4 walls, 35–50% infill |
| Wind vane | Vane, shaft hub, AS5600 enclosure | PETG/ASA, 4 walls, 30–40% infill |
| Rain gauge | Funnel, bucket, pivots, enclosure | PETG/ASA, 4 walls, 30–40% infill |

Use a 0.4 mm nozzle and 0.2 mm layer height unless the model specifically recommends otherwise. For parts that must be watertight, use at least four perimeter walls and inspect layer seams before installation.

## Main pole mount

The documented pole clamp is intended for a **3/4-inch galvanized steel pipe** and uses M8 clamping hardware. The project notes that the printed M8 threads should be chased with an M8 tap so the bolts tighten cleanly and hold the base against rotation in wind. 

1. Remove any brim or elephant foot from the two clamp halves and the pipe bore.
2. Run an M8 tap through every printed threaded hole. Do not force a bolt through untreated printed threads.
3. Insert the M8 bolts by hand until they pass freely through the threads.
4. Place the clamp around the 3/4-inch pipe at a comfortable work height.
5. Tighten both bolts alternately until the clamp is secure and square; do not crush the printed plastic.
6. Fit the arm sockets or arm base to the pole mount and verify that they point away from each other at the intended orientation.
7. Perform final tightening only after all arms and sensors are installed, so you can set a level, balanced orientation.

## Electronics enclosure

The enclosure holds the NodeMCU, power wiring, I2C distribution, and the connections for rain and wind pulse sensors. The station runs from 5 V on the NodeMCU Vin input, with a documented maximum system draw of approximately 500 mA.

1. Fit cable glands or grommets into the enclosure openings before mounting electronics.
2. Mount the ESP8266 NodeMCU on its printed posts, using small screws or a non-corrosive adhesive pad as appropriate.
3. Install a small terminal block, I2C distribution board, or soldered wiring harness with enough strain relief for the sensor cables.
4. Route power separately from pulse-sensor leads where possible to reduce electrical noise.
5. Connect all I2C modules in parallel: SDA to GPIO 4/D2 and SCL to GPIO 5/D1.
6. Connect the anemometer Hall-sensor output to GPIO 12/D6 and the rain-gauge Hall-sensor output to GPIO 14/D5.
7. Keep all sensor grounds common with the NodeMCU ground.
8. Before closing the box, power the unit by USB and verify boot, Wi-Fi setup, and I2C detection.
9. Fit a drip loop below every cable entry, then close the enclosure with its gasket or a thin bead of neutral-cure silicone if necessary.

Do **not** fill the electronics enclosure completely with silicone: you need future access to the ESP8266 FLASH button, wiring, and service points. GPIO 0/D3 is used for the documented 10-second factory-reset function.

## Temperature, humidity, pressure, air-quality and light arm

The environmental hardware consists of the AHT20/AHT21, BMP280, ENS160, BH1750, and AS5600 on the shared I2C bus. Their standard addresses are documented as AHT20 `0x38`, BMP280 `0x76` or `0x77`, ENS160 `0x52` or `0x53`, AS5600 `0x36`, and BH1750 `0x23`.

1. Assemble the radiation-shield stack in the order shown by the 3MF/STL design: bottom cap, lower louvres, sensor carrier, upper louvres, and top rain cap.
2. Install the AHT20/AHT21 and BMP280 in the shield's protected central airflow area.
3. Ensure neither sensor touches a sun-exposed wall of the shield.
4. Route the I2C cable upward or sideways with a drip loop rather than straight into the top of the sensor chamber.
5. Mount the ENS160 in its dedicated ventilated enclosure or sensor cavity; it should be protected from direct rain but exposed to ambient air.
6. Install the BH1750 behind its intended transparent PETG cover, with the sensor face aimed upward and free of overhang shadows.
7. Mount the assembled environmental arm to the main hub so it is clear of the rain funnel and rotating wind parts.

The project specifically warns that too many I2C pull-ups in parallel can make the bus pull-up resistance too low; it recommends removing the SDA and SCL pull-up resistors from the BMP280 module if required. It also notes a potential ENS160 humidity-sensor/address conflict that may require modifying that module as documented by the project.

## Radiation shield orientation

The radiation shield is intended to keep direct solar radiation and rain off the temperature and humidity sensor while allowing airflow.

- Keep the shield's louvre openings unobstructed.
- Install it at least several centimetres away from the electronics box, pole, and any large printed surfaces that may warm in sunlight.
- Prefer a light-coloured, ideally white, shield.
- Orient any cable exit downward.
- Do not place the temperature sensor directly below a roof edge, chimney, air-conditioning outlet, or reflective wall.

## Wind vane

The AS5600 is used as a contactless magnetic rotary encoder for wind direction, with the firmware calculating direction from its I2C angle reading. The station provides a **Calibrate North** action that stores a software direction offset after you physically point the vane north.

1. Degrease the 608Z bearing using the project's two 15-minute IPA baths, then let it dry completely.
2. Add only a few drops of light machine or gun oil after drying; excess oil attracts dirt.
3. Press the bearing into the vane housing squarely. Apply force only to the bearing's outer race.
4. Insert the vane shaft through the bearing and verify that it rotates freely without binding.
5. Fit the magnet into the vane shaft or magnet holder with the polarity and axial alignment expected by the AS5600 mount.
6. Install the AS5600 PCB in its fixed housing, centred beneath or above the magnet as the model indicates.
7. Verify that the magnet rotates without rubbing the PCB, cover, or sensor housing.
8. Attach the vane fin and insert the counterweight into the vane tip.
9. Adjust the counterweight until the vane remains approximately horizontal and settles smoothly.
10. Seal the AS5600 housing from rain while retaining enough clearance for free rotation.

Keep the magnetic encoder away from steel brackets, loose magnets, and high-current power wiring where practical. Before final mounting, check that the web UI reports a changing direction value over a full vane rotation.

## Anemometer

The anemometer uses an A3144 Hall sensor on GPIO 12/D6 and one or more rotor magnets to generate pulses. Its calibration depends on cup radius, number of magnets, and aerodynamic factor; the documented defaults are 80 mm radius, one magnet, and a factor of 3.0.

1. Prepare the bearing as described for the vane: degrease, dry, then lightly oil.
2. Press the bearing into the anemometer body and insert the vertical shaft.
3. Assemble and attach the cups or cup arms to the rotor hub, ensuring all cups face the same rotational direction.
4. Measure from the centre of the shaft to the centre of each cup; set the firmware radius to the actual measured value, not merely the nominal print dimension.
5. Insert the rotor magnet or magnets in their designed pockets, ensuring they are secure and evenly spaced.
6. Install the A3144 Hall sensor in its holder at the designed distance from the magnet path.
7. Route the sensor cable down the arm with slack near the rotating assembly but no possibility of contact with the rotor.
8. Turn the rotor by hand and confirm that it spins with minimal friction and does not wobble.
9. Power the device and watch the diagnostic console or dashboard while turning the rotor; verify that pulse count or wind speed changes.
10. Fasten the finished anemometer above nearby structures and arms so wind reaches the cups cleanly.

Use the software wind debounce setting to suppress switch bounce or double-counting. The project's default is 15 ms, while the wind sample interval is independent of the environmental I2C polling interval. 

## Rain gauge

The rain gauge is a modified tipping-bucket design using an A3144 Hall sensor on GPIO 14/D5. The documented default conversion is 0.6314 mm per tip, and the project describes two magnets in the bucket assembly for reliable pulse triggering. 

1. Assemble the rain funnel, collection throat, and bucket housing according to the printed part geometry.
2. Fit the tipping bucket onto its pivot points or axle and ensure it moves freely from one side to the other.
3. Install the two magnets in the bucket's designated pockets.
4. Mount the A3144 Hall sensor in the fixed sensor holder, close enough to detect each passing magnet but not close enough to touch during motion.
5. Connect the sensor cable to GPIO 14/D5, ground, and the appropriate supply as required by the sensor module.
6. Confirm that the funnel drains solely into one side of the bucket and that water cannot bypass the tipping mechanism.
7. Check that the bucket returns fully to its resting position after each tip.
8. Mount the rain gauge level using a small bubble level; a tilted gauge produces inaccurate rainfall totals.
9. Keep the funnel unobstructed and above local splash-back surfaces.

### Bucket calibration

The repository recommends calibrating each bucket side independently with 6 ml of water. 

1. Reset the station's rain total or record the starting tip count.
2. Slowly pour exactly 6 ml into one side of the bucket.
3. Adjust that side's calibration screw until the bucket tips and drains at that volume.
4. Repeat with 6 ml on the opposite side.
5. Repeat both sides several times until they tip consistently.
6. Pour a larger measured volume through the funnel at a realistic rainfall rate and compare observed tips with the expected total.
7. Update the **mm per tip** setting if field testing shows a consistent difference from the default.

Do not test by pouring water rapidly: a tipping bucket can undercount when water enters faster than the mechanism can cycle.

## Final outdoor layout

Fit the components in this order to avoid repeatedly dismantling the station:

1. Mount the main clamp and arms on the pole.
2. Install the radiation shield and environmental sensors.
3. Install the electronics enclosure below or to the sheltered side of the arms.
4. Install the rain gauge level and clear of the pole's drip path.
5. Install the wind vane and anemometer at the highest point.
6. Route and secure cables with UV-resistant cable ties, leaving service loops near each sensor.
7. Ensure cables cannot rub against rotating assemblies or form water traps.
8. Recheck every fastener after the first week outside and again after the first high-wind event.

For the most meaningful wind observations, position the wind instruments as high and unobstructed as practical; the rain gauge should be level and away from nearby surfaces that create splash, turbulence, or wind shadows.

## Commissioning checklist

After mechanical assembly, verify the station before leaving it unattended:

- The NodeMCU boots and joins Wi-Fi or exposes its setup AP.
- All intended I2C sensors are detected.
- Temperature, humidity, pressure, AQI, light, wind speed, wind direction, and rain values change plausibly.
- Wind-direction value covers the full 0–359-degree range without dropouts.
- **Calibrate North** is run with the vane physically aligned to true north.
- The rain bucket increments exactly once per manual tip.
- The anemometer produces pulses during hand rotation.
- The enclosure remains dry after a hose or light-spray test.
- MQTT telemetry appears on `tele/<hostname>/SENSOR` if Home Assistant integration is enabled.
- Backup configuration is downloaded once the station is fully calibrated.

## Final note
**Important:** These assembly instructions are a practical guide. Refer to the MakerWorld 3MF/STL object names and fit geometry for the exact arrangement of printed pieces. Dry-fit all components before gluing or permanently sealing anything, since printer tolerances, filament shrinkage, bearing dimensions, and sensor-board layouts can vary. [3D models on makerworld](https://makerworld.com/en/models/3139553-fully-optional-smart-weather-station)
