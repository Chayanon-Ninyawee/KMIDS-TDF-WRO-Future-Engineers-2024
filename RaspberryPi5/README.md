# How to Setup RaspberryPi5
- Insert a microSD card, which should contain 8GB or larger into computer.
- Download, install and run [Raspberry Pi Imager](https://www.raspberrypi.com/software/) that is appropriate for your operating system.
- Click the choose device button and a list of Pi boards appears.
 
<img src="https://cdn.mos.cms.futurecdn.net/SPUHaV6g8Qxc2yBNd6qFW8-970-80.png" width="400" />

- Choose Raspberry Pi 5 in the Pi board. 
  
<img src="https://robu.in/wp-content/uploads/2024/01/o7.jpg" width="500" />

- Click the Choose OS button and menu will appear. 

<img src="https://cdn.discordapp.com/attachments/1196726335867998269/1308708929718714389/image.png?ex=673eedab&is=673d9c2b&hm=cb79c6e8f50fe94e3aea4d146fa50add4f7151fb0ff35a123769ea3f42c9172f&" width="400" />

- Choose the top choice, Raspberry Pi OS (64-bit), for Pi 5. If there are Raspberry Pi OS on top choice, click that and choose the latest version that works with your board.

<img src="https://robu.in/wp-content/uploads/2024/01/o6.jpg" width="500" />

- Click Choose Storage and select the card from the menu.

<img src="https://robu.in/wp-content/uploads/2024/01/o8.jpg" width="500" />

- Click Next 

<img src="https://cdn.mos.cms.futurecdn.net/cQHK7tWkKGRENVuMkR5Gkg-970-80.png.webp" width="400" />

- Click Edit Settings 

<img src="https://cdn.discordapp.com/attachments/1196726335867998269/1308709818537742407/image.png?ex=673eee7f&is=673d9cff&hm=0e207f68b3dde950df18f7399f9416c38dd36799c61f329f531dafe86e39a99c&" width="400" />

- Fill in all blank on the General tab: hostname, username / password, wireless LAN (If you plan to use Wi-Fi, and locale settings).

<img src="https://cdn.mos.cms.futurecdn.net/Et4hHahUd3dN3nufsLKqFN-970-80.png.webp" width="400" />

- Go to Serivec Tab, Click Enable SSH button and Select "Use password authentication."  Then click Save.

<img src="https://cdn.mos.cms.futurecdn.net/FQPA4pWp9qswNM8feDE4ye-970-80.png.webp" width="400" />

- Click Yes to apply OS customization settings. !

<img src="https://cdn.mos.cms.futurecdn.net/z3SSm8nAART9rhkdxr3jvk-970-80.png.webp" width="400" />

- Click Yes to confirm that you want to write to your microSD card. !

<img src="https://cdn.mos.cms.futurecdn.net/4WM6JqmAUGPpmXJxqMzfC6-970-80.png.webp" width="400" />

- It will take a few minutes to download the OS and write it to the card. 

<img src="https://cdn.mos.cms.futurecdn.net/upTCsdvyixsdfuyhtyKJdQ-970-80.png.webp" width="400" />

- Once the process is complete, insert the microSD card into your Raspberry Pi, power it on, and give it a few seconds to connect to the network. Then, you can try logging in via SSH, assuming both the Raspberry Pi and your computer are connected to the same Wi-Fi network.