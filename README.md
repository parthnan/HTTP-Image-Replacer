[![HitCount](http://hits.dwyl.com/parthnan/HTTP-Image-Replacer.svg)](http://hits.dwyl.com/parthnan/HTTP-Image-Replacer)
# Overview
Modified an Open Source HTTP Proxy software, ffproxy(http://ffproxy.sourceforge.net/), to make the browser edit image tags in HTTP response bodies to display any image online or on computer(Not possible with encoded HTTPS). It replaces the URLs of all images found in HTML part of successful (code 200) responses, with the specified URL (within pne of the modified files ffproxy-1.6/request.c).

Motivation : This modified software enables users to replace images on specific blacklisted sites with specified image, allowing content blocking and a smoother web experience.

# Test Run
Usage method below image. It completely changed my msn home page! All images replaced by this image:https://images.pexels.com/photos/18495/pexels-photo.jpg?auto=compress&cs=tinysrgb&h=3X5  . 
![alt text](https://raw.githubusercontent.com/parthnan/HTTP-Image-Replacer/master/replaced.png)

# How To Use
First download and install Image replacer by the commands shown below.

Run the modified ffproxy using the following commands on a UNIX machine(in my case a VM on my localhost), which will be the Proxy machine in between the server(websites,Browser) and client(my computer OS). Within the code, port 8080 is specified as the proxy socket.

Set the browser of client machine to connect to port <Proxy machine IP>:8080 not the default 80, for example I set options for Edge as below.
