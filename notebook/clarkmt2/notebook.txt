# Water Blaster Development Log

## Week of January 31st

Developed first version of our board very early, we are unsure of mechanical implementation just yet

Few ideas we have:
- Plunger
- Expandable ballast
- Pressurize water with resovoir
- Accumulator tank
- Normal tank with bleed valve

**January 31st**  
Purchased parts for all of these and will use V1 to test

---

## Week of February 20th

Tested all of the above with the initial board (no pressure sensor so used gauge, accumulator tank is by far the best solution but leaks slightly.  
Water was so powerful it bounced off the wall and landed on the board, shorted MCU and board died slightly after (RIP)

- Ordered parts and began CAD Design, on track for everything, multiple new PCBS designed for trigger
- Added step encoder to adjust menus, designed dpad and a few other things
- Made 1 single connector to IO board - cuts the costs a significant amount.
- Changed to screw terminals because a ticker wire guauge was needed
- Used 5V power for the encoder, and the pressure transducer, it paid off adding this to the board early
- New revision has pins rearranged to optimize layout and reduce total number of vias
- Added 3 leak detection subboards that can be distributed throughout the board

---

## Leak Detection Subsystem

These boards have the GPIO with a pullup resistor, and a sensor pin. The sensors pins are interlaced with GND pins.  
The idea is that if water comes across the pins the sensor pins will drop to GND as the water connects it, meaning our GPIO would be active low for a leak being detected. This is our first design for a leak sensor and we have yet to test it.  
We decided on 3 sensors:
- One directly near the IO board
- One directly near the fill valve
- One on the bottom of the enclosure for when it is hold in normal firing position

The hope is that with these 3 locations we would get a splash of water before it would be possible to reach the microcontroller, minimizing the possible damage to the system.

---

## Enclosure Design and Sketch

We also have a very basic sketch that I have attached to my notebook that shows our ideal end enclosure.

After continuing to work on the enclosure with the finalized parts we have determined that the water blaster will look pretty good.  
I have attached photos of our CAD into the notebook on my section.

---

## Design Procedure Overview

When we began our design we found many different potential solutions, some we were able to rule out quickly due to lack of part availability, such as an expandable water bladder to attach to our pump, as when we went to work on this, we found it would need a custom rubber part to be manufactured and they had an order minimum of 100 units. As a result, that method was ruled out. However, we were left with many feasible configurations just between the pump, valve, tank, and nozzle, which we needed to test ourselves as we found no effective way to model these components due to a lack of availability of technical specifications and performance.  

As for electrical design, we completed our first iteration of the board quickly because we wanted to be able to control various mechanical components, specifically the pump and valve subsystems. We knew that these would be crucial for our final design so evaluating what would be feasible early on was essential. With our initial PCB design, we were able to successfully control the pump and valve using an N-Channel MOSFET. This allowed us to test various pumps, such as self-priming pumps, diaphragm pumps, low voltage pumps, and so on. We also tested various water holding methods, such as reservoirs, expandable bladders, accumulator tanks, and no large water method other than the pipes. Using our early iteration of the PCB we managed to narrow down what would work best, in terms of tanks. The accumulator tank consistently outperformed the other methods and allowed for easy filling with minimal parts. We also found that the water reservoir worked quite well, but was more expensive, and had a special air bleeding valve that was required to remove air from the system, which was quite costly. This also puts a restriction on the orientation of the gun for firing which we believed would negatively impact the end users experience.  
Given that we had established the accumulator tank as the superior method we then began evaluating the pumps. We determined that threaded pumps worked best for our use, although the assembly became a bit more tedious as opposed to hose clamp pumps, the threaded fittings virtually eliminated all the internal leaks which was a huge benefit for preventing water ingress. With the main components of the water blaster finalized we were able to evaluate different nozzles. We started with a pressure washer nozzle, thinking that would be the ideal nozzle, but it ended up missing the water after 5-10 ft, which did not meet our high-level criteria. We found that a 1/8 in diameter nozzle ended up working best as the water clumped together increasing the firing distance significantly.  

---

## Sensor and I/O Design Decisions

In terms of the sensors, we had to decide between a few different things, we found commercially available water sensors, but we were unhappy with the formfactor, ultimately, we decided to design our own to know the exact dimensions, ensure connector compatibility, and ultimately ensure it would fit inside our enclosure where we would like it to go.  
For the I/O board we also needed to have a knob to adjust the settings menu, we initially planned to use a potentiometer hooked up to our ADC, but then realized that this would have limits in our settings menu, and jump to whatever value the potentiometer was at when the page was opened, so we instead switched to a step encoder.  
This allowed the encoder to spin all the way around every time, and instead we tracked the relative change in the encoder position to adjust the settings value.  

Finally, we had to decide on a water level monitoring system.  
We knew we wanted to monitor pressure for safety, and with a working mechanical enclosure we were able to see that after 25 psi the water distance started to decrease significantly.  
As a result, instead of tracking the quantity of water, we tracked the pressure of the water. This makes more sense as the pressure determines what is a suitable shot rather than the amount of water inside of the water blaster.  
The other options we evaluated for this were flow sensors and ultrasonic sensors, both would be costly and unable to evaluate the exact number of usable shots remaining as the water out does not directly correlate to what is a usable shot inside of the tank which is why pressure monitoring ended up coming out on top.  

---

## Frame and Shell Details

The frame and shell of the water blaster provide structural support and protection for the internal components. It prevents water ingress and ensures that all components are securely housed.  
The frame is made from 3D-printed or composite materials, which offer a lightweight yet durable solution. TPU-sealed buttons and NPT fittings are incorporated to prevent leaks.  
A conformal coating protects the electronics from potential moisture exposure.  

Ultimately the mechanical components that provided the most powerful water bursts required a significant amount of research and development.  
We tested many different configurations ranging from a direct pump to nozzle layout, a pump to expandable bladder to nozzle layout and a pump to accumulator tank to nozzle layout.  
Ultimately the accumulator tank proved to be the superior method.  

The accumulator tank is charged with a pressure of roughly 5psi of air, which the pump overcomes with its suction of water, increasing the pressure past this, as this happens the air becomes more and more compressed because its volume decreases, essentially increasing the pressure and putting a force behind the water, helping maintain higher pressure for longer.  

As seen in the picture attached to my lab notebook, the inlet goes to the pump, which then pumps water into the accumulator tank where it is stored in a pressurized state, there is then a pressure transducer inline on the barrel which records the pressure and reports it out to the main board.  

Right before the nozzle there is a solenoid valve. This valve is capable of opening and closing very quickly, releasing powerful bursts of water for whatever duration is configured in the firmware.  

After component selection was completed, we began optimizing the enclosure.  
We placed mounting holes for all of our boards, such as leak sensors, trigger boards, the IO Board, and Main Board. This ensures that everything is mounted properly and stays exactly where we want it.  

We also opted for an asymmetrical parting line in the design, allowing all for a functional water blaster with only half of an enclosure, meaning all components can be installed into the left side of the shell, and the right side is removable to see what is going on inside.  

---

## Final Assembly and Presentation

The Final mechanical enclosure came out to look exactly how we expected.  
We were able to clean everything inside and take advantage of TPU button covers to prevent water ingress.  
The final assembled water blaster can be seen below.  

Ultimately we are very happy with our final project, the demonstration went well with only minor hiccups along the way.  
We had some issues with the trigger button getting stuck but we were able to diagnose this as issues with the spacer washers on the trigger PCB.  
We resolved this before recording our extra credit video and uploading it to YouTube.  
We went on to present our project and everything went very well.

---

## 🎥 Extra Credit Video

**Link:** [https://www.youtube.com/watch?v=EbqiNhb4QeM](https://www.youtube.com/watch?v=EbqiNhb4QeM)

---

## 📚 References

[1] “Bernoulli’s Equation.” Princeton University  
https://www.princeton.edu/~asmits/Bicycle_web/Bernoulli.html  

[2] “Elastic Pressure Water Blaster Technology.” iSoaker  
https://www.isoaker.com/Tech/elastic-pressure-water-blaster-technology.php  

[3] “How to Manufacture Carbon Fiber Parts.” Formlabs  
https://formlabs.com/blog/composite-materials-carbon-fiber-layup/  

[4] Lam, L. (n.d.). The continuous refill, short-burst, hand-powered water toy.  
https://dspace.mit.edu/bitstream/handle/1721.1/59943/676918672-MIT.pdf  

[5] Rathi, S. (2024, July 11). A critical review of leakage detection strategies including pressure and water quality sensor placement in water distribution systems – sole and integrated approaches for leakage and contamination intrusion.  
https://riunet.upv.es/handle/10251/205921
