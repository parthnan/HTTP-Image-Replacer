# Overview
Modified an Open Source HTTP Proxy software, ffproxy(http://ffproxy.sourceforge.net/), to make the browser edit image tags in HTTP response bodies to display any image online or on computer(Not possible with encoded HTTPS). It replaces the URLs of all images found in HTML part of successful (code 200) responses, with the specified URL (within pne of the modified files ffproxy-1.6/request.c).

# Test Run
It completely changed my msn home page! All images replaced by this image:https://images.pexels.com/photos/18495/pexels-photo.jpg?auto=compress&cs=tinysrgb&h=3X5  . However it is buggy with some other HTTP sites, progress ongoing.

![alt text](https://raw.githubusercontent.com/parthnan/HTTP-Image-Replacer/master/replaced.png)

