# http-related-attacks

HTTP Related Attacks

1.	HTTP GET Flood Attack
•	HTTP-GET flood attack threats have increased day by day. 
•	In this type of attacks, malicious clients send numerous HTTP-GET requests to the destination Web server automatically. 
•	Since these HTTP-GET requests have legitimate formats and are sent via normal TCP connections, an intrusion detection system (IDS) cannot detect them [6]. 
•	In this research project, HTTP-GET flood detection technique is based on keeping a track of GET packets count for same source-destination by parsing the header information using a click element developed in clickOS.
•	Once the count reaches the threshold, anomaly is detected!

2.	Host Intrusion Detection System (URI Detector for /etc/passwd) [1]
•	Traditionally, the /etc/passwd file is used to keep track of every registered user that has access to a system. The /etc/passwd file is a colon-separated file that contains the following information: User name. Encrypted password. User ID number (UID), etc [8].
•	The GET Packet location in header starts at 54th byte with hex value: 474554
•	The Request URI in the GET Packet in header starts at 58th byte with hex value: 2F which is hex URL encoding for ‘/’
 
•	If the URI is /etc/passwd, that is considered an attack as it is a file path that has all users’ confidential data like credentials (username and password).
•	/etc/passwd can be encoded in escaped Hex format as /%65%74%63/%70%61%73%73%77%64
•	To detect this attack, a host intrusion detection system is required and using the click code, attack can be detected.
 
 

References
[1] http://www-inst.cs.berkeley.edu/~cs161/fa16/slides/monitoring1.key.pdf
[2] http://scriptasylum.com/tutorials/encode-decode.html
[3] https://www.eso.org/~ndelmott/url_encode.html
[4] https://ascii.cl/
[5] https://blog.radware.com/security/2017/11/http-attacks/
[6] https://ieeexplore.ieee.org/document/4313218/
[7] K. J. Singh and T. De, "DDOS Attack Detection and Mitigation Technique Based on Http Count and Verification Using CAPTCHA," 2015 International Conference on Computational Intelligence and Networks, Bhubaneshwar, 2015, pp. 196-197.
doi: 10.1109/CINE.2015.47
URL: https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=7053830&isnumber=7053782
[8] https://www.google.com/search?q=%2Fetc%2Fpsswd&rlz=1C1CHBF_ enUS723US724&oq=%2Fetc%2Fpsswd&aqs=chrome..69i57j0j69i58j0l3.2708j0j7&sourceid=chrome&ie=UTF-8






