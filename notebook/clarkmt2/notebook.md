First main board is ready around week 5. 

We met with Jack Blevins
Developed first version of our board very early (before week 8), we are unsure of mechanical implementation just yet

Few ideas we have,
Plunger, expandable ballast
Pressurize water with resovoir
Accumulator tank
Normal tank with bleed valve

January 31st
Purchased parts for all of these and will use V1 to test

February 20th
Tested all of the above with the initial board (no pressure sensor so used gauge, accumulator tank
is by far the best solution but leaks slightly
water was so powerful it bounced off the wall and landed on the board, shorted MCU and board died
slightly after (RIP)

Ordered parts and began CAD Design, on track for everything, multiple new PCBS designed for trigger
added step encoder to adjust menus, designed dpad and a few other things

made 1 signle ocnnector to IO board - cuts the costs a significant amount.

Changed to screw terminals because a ticker wire guauge was needed

used 5V power for the encoder, and the pressure transducer, it paid off adding this to the board early
new revision has pins rearranged to optimize layout and reudce total number of vias

added 3 leak detection subboards that can be distributed throughout the board

these boards have the GPIO with a pullup resistor, and a sensor pin, the sensors pins are interlaced
with GND pins, the idea is that if water comes across the pins the sensor pins will drop to GND as
the water connects it meaning our GPIO would be active low for a leak being detected. This is our
first design for a leak sensor and we have yet to test it.
We deicded on 3 sensors, one directly near the IO board, one directly near the fill valve, and one on
the bottom of the enclosure for when it is hold in normal firing position. The hope is that with these 3
locations we would get a splash of water before it would be possible to reach the microcontroller
minimizing the possible damage to the system

We also have a very basic sketch that I have attached to my notebook that shows our ideal end enclosure.

After continuing to work on the enclosure witht he finalized parts we have determined that the water blaster will look pretty good. I have attached photos of our CAD into to the notebook on my seciton. 


March 11th - April 20th
Design procedure overview (similar to what will be in the report)

This section mentions adjustments from weekly TA meeting checks and how we got guided to our final solution. 

When we began our design we found many different potential solutions, some we were able to
rule out quickly due to lack of part availability, such as an expandable water bladder to 
attach to our pump, as when we went to work on this, we found it would need a custom 
rubber part to be manufactured and they had an order minimum of 100 units. As a result, 
that method was ruled out. However, we were left with many feasible configurations just 
between the pump, valve, tank, and nozzle, which we needed to test ourselves as we found 
no effective way to model these components due to a lack of availability of technical 
specifications and performance.  


As for electrical design, we completed our first iteration of the board quickly because we 
wanted to be able to control various mechanical components, specifically the pump and 
valve subsystems. We knew that these would be crucial for our final design so evaluating
what would be feasible early on was essential. With our initial PCB design, we were able 
to successfully control the pump and valve using an N-Channel MOSFET. This allowed us to 
test various pumps, such as self-priming pumps, diaphragm pumps, low voltage pumps, and so
on. We also tested various water holding methods, such as reservoirs, expandable bladders,
accumulator tanks, and no large water method other than the pipes. Using our early 
iteration of the PCB we managed to narrow down what would work best, in terms of tanks. 
The accumulator tank consistently outperformed the other methods and allowed for easy 
filling with minimal parts. We also found that the water reservoir worked quite well, but
was more expensive, and had a special air bleeding valve that was required to remove air 
from the system, which was quite costly. This also puts a restriction on the orientation 
of the gun for firing which we believed would negatively impact the end users experience.
Given that we had established the accumulator tank as the superior method we then began 
evaluating the pumps. We determined that threaded pumps worked best for our use, although
the assembly became a bit more tedious as opposed to hose clamp pumps, the threaded 
fittings virtually eliminated all the internal leaks which was a huge benefit for 
preventing water ingress. With the main components of the water blaster finalized we were
able to evaluate different nozzles. We started with a pressure washer nozzle, thinking 
that would be the ideal nozzle, but it ended up missing the water after 5-10 ft, which did
not meet our high-level criteria. We found that a 1/8 in diameter nozzle ended up working 
best as the water clumped together increasing the firing distance significantly. 


In terms of the sensors, we had to decide between a few different things, we found 
commercially available water sensors, but we were unhappy with the formfactor, ultimately,
we decided to design our own to know the exact dimensions, ensure connector 
compatibility, and ultimately ensure it would fit inside our enclosure where we would 
like it to go. For the I/O board we also needed to have a knob to adjust the settings 
menu, we initially planned to use a potentiometer hooked up to our ADC, but then realized
that this would have limits in our settings menu, and jump to whatever value the 
potentiometer was at when the page was opened, so we instead switched to a step encoder.
This allowed the encoder to spin all the way around every time, and instead we tracked 
the relative change in the encoder position to adjust the settings value. Finally, we had
to decide on a water level monitoring system. We knew we wanted to monitor pressure for 
safety, and with a working mechanical enclosure we were able to see that after 25 psi the
water distance started to decrease significantly. As a result, instead of tracking the 
quantity of water, we tracked the pressure of the water. This makes more sense as the 
pressure determines what is a suitable shot rather than the amount of water inside of the
water blaster. The other options we evaluated for this were flow sensors and ultrasonic 
sensors, both would be costly and unable to evaluate the exact number of usable shots 
remaining as the water out does not directly correlate to what is a usable shot inside of
the tank which is why pressure monitoring ended up coming out on top.  



Frame and shell details


The frame and shell of the water blaster provide structural support and protection for 
the internal components. It prevents water ingress and ensures that all components are 
securely housed. The frame is made from 3D-printed or composite materials, which offer a
lightweight yet durable solution. TPU-sealed buttons and NPT fittings are incorporated to
prevent leaks. A conformal coating protects the electronics from potential moisture 
exposure. Ultimately the mechanical components that provided the most powerful water 
bursts required a significant amount of research and development. We tested many 
different configurations ranging from a direct pump to nozzle layout, a pump to 
expandable bladder to nozzle layout and a pump to accumulator tank to nozzle layout. 
Ultimately the accumulator tank proved to be the superior method. The accumulator tank is
charged with a pressure of roughly 5psi of air, which the pump overcomes with its suction
of water, increasing the pressure past this, as this happens the air becomes more and 
more compressed because its volume decreases, essentially increasing the pressure and 
putting a force behind the water, helping maintain higher pressure for longer. As seen in
the picture attached to my lab notebook, the inlet goes to the pump, which then pumps 
water into the accumulator tank where it is stored in a pressurized state, there is then
a pressure transducer inline on the barrel which records the pressure and reports it out
to the main board. Right before the nozzle there is a solenoid valve. This valve is 
capable of opening and closing very quickly, releasing powerful bursts of water. For 
whatever duration is configured in the firmware. After component selection was completed,
we began optimizing the enclosure. We placed mounting holes for all of our boards, such 
as leak sensors, trigger boards, the IO Board, and Main Board. This ensures that 
everything is mounted properly and stays exactly where we want it. We also opted for an
asymmetrical parting line in the design, allowing all for a functional water blaster with
only half of an enclosure, meaning all components can be installed into the left side of
the shell, and the right side is removable to see what is going on inside.  



The Final mechanical enclosure came out to look exactly how we expected. We were able to
clean everything inside and take advantage of TPU button covers to prevent water ingress.
The final assembled water blaster can be seen below. 



Ultimately we are very happy with our final project, the demonstration went will with 
only minor hiccups along the way. We had some issues with the trigger button getting 
stuck but wew ere able to diagnose this as issues with the spacer washers on the trigger 
PCB. We resolved this before recording our extra credit video and uploading it to 
youtube. We went on to present our project and everything went very well.


Link to extra credit video and demonstration of our project.

https://www.youtube.com/watch?v=EbqiNhb4QeM

April 21st - May 6th

Subsystem 

Requirement 

Verification 

Control Board 

The control board must process inputs and update outputs within 100 ms of receiving sensor data. 

 

The control board must activate the pump and solenoid valve correctly 100% of the time triggered. 

 

The control board must correctly send data to the SPI display without noticeable glitches or missing updates. 

Observe system behavior with button presses and sensor inputs to ensure there is no noticeable lag. 

 

Run 50 activation cycles and verify that all commands result in the expected action. 

 

Operate the system for 5 minutes, ensuring that display information updates smoothly without flickering or freezing. 

Inputs & Outputs 

The trigger button must activate the water blaster every time it is pressed, with no missed input.  

 

The potentiometer must allow smooth control over firing power with noticeable differences between minimum and maximum settings. 

Press the button 50 times and confirm that every press result in activation of some sort 

 

Adjust the potentiometer and confirm that different power levels produce visibly different water blasts, the water quantity can be measured using a beaker to ensure the quantity of water fired has increased. 

 

Battery 

The battery must power the system for at least 30 minutes of continuous firing operation. 

 

The battery system must provide a stable 12 V (1 V margin of error) output. 

Fully charge the battery, operate the system continuously, and record runtime on a stopwatch. 

 

Measure voltage before and after 10 minutes of operation to ensure no significant drop. 

Frame & Shell 

The enclosure must prevent leaks when sprayed with water from all angles for 5 minutes. 

 

The shell must not crack or deform if dropped from 1 meter onto grass or pavement. 

Spray tests the assembled device and inspect for internal moister. Ensure a maximum of 1.5 ml of water ingress. 

 

Drop test from 1 meter height and confirm no functional damage. 

Sensor Array 

The system must accurately indicate the capacity of the tank within 25%. 

 

The system must alert the user within 10 seconds of detecting internal water leakage.  

Fill and empty the tank to these levels and confirm the display updates correctly with the indicated water level after water in/water out is measured.  

 

Pour 100 ml of water into the system, hold the water blaster vertically as if in firing position. The system should shut down within 10 seconds of the blaster being held vertically or never power on to begin with. Time with a stopwatch to ensure timely power down sequence.  

Pump, Solenoid Valve, and Tank 

The pump must fill the tank from empty within 30 seconds. 

 

The valve must open and close within 100 ms of being triggered.  

 

The blaster must shoot water at least 15 ft when fully pressurized. For at least one running configuration, a maximum shot distance of 20 ft should be achieved. 

Time how long it takes to fill the tank and ensure it meets the requirement using a stopwatch. Ensure the tank is completely empty prior to this test.  

 

Fire the blaster and use a stopwatch to tie the delay between button press and water visibly leaving the blaster. It should be within 100 ms. 

 

Fire the blaster and measure the distance reached. Tune to achieve 5 powerful bursts and record the distance to ensure 15 ft is recorded. Then refill and tun the settings for the maximum power and fire one shot to ensure 20 ft is recorded.  



References:

[1] “Bernoulli’s Equation.” Princeton University, The Trustees of Princeton University, www.princeton.edu/~asmits/Bicycle_web/Bernoulli.html. Accessed 6 Mar. 2025.  

 

[2] “Elastic Pressure Water Blaster Technology .:” Elastic Pressure Water Blaster 
Technology :: :: iSoaker.Com, www.isoaker.com/Tech/elastic-pressure-water-blaster-
technology.php. Accessed 6 Mar. 2025.  


 

[3] “How to Manufacture Carbon Fiber Parts.” Formlabs, formlabs.com/blog/composite-materials-carbon-fiber-layup/. Accessed 6 Mar. 2025. 

 

[4] Lam, L. (n.d.). The continuous refill, short-burst, hand- powered water toy. https://dspace.mit.edu/bitstream/handle/1721.1/59943/676918672-MIT.pdf?sequence=2  

 

[5] Rathi, S. (2024, July 11). A critical review of leakage detection strategies including pressure and water quality sensor placement in water distribution systems – 
sole and integrated approaches for leakage and contamination intrusion. 2nd International
Join Conference on Water Distribution System Analysis (WDSA) & Computing and Control in 
the Water Industry (CCWI). https://riunet.upv.es/handle/10251/205921  
