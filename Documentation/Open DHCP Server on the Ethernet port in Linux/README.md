## How to connect to Raspberry pi 5 via ethernet cable (Open DHCP Server on the Ethernet port in Linux)
- ### Select interface with:
  ```
  sudo nano /etc/default/isc-dhcp-server
  ---
  INTERFACESv4="eno1"
  INTERFACESv6=""

  sudo nano /etc/dhcp/dhcpd.conf
  ---
  subnet 192.168.39.0 netmask 255.255.255.0 {
  range 192.168.39.10 192.168.39.50;
  option subnet-mask 255.255.255.0;
  option routers 192.168.39.1;
  }
  ```

- ### Add new Ethernet Connection
  make the ipv4 address the same as above (192.168.39.1)
- ### How to run:
  ```
  sudo systemctl start isc-dhcp-server.service
  ```
- ### How to check:
  ```
  sudo systemctl status isc-dhcp-server.service
  ```
- ### If error:
  ```
  journalctl _PID=<PID Here>
  ```
- ### Don't forget to make sure the interface is up:
  ```
  ip addr show eno1
  ```
- ### How to autostart:
  ```
  sudo systemctl enable isc-dhcp-server.service
  ```
- ### How to stop autostart:
  ```
  sudo systemctl disable isc-dhcp-server.service
  ```
