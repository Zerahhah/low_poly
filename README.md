# low_ploy
## result
### input_image
![](input_image2.jpg)
### result_image
![](result_image2.jpg)
### input_image
![](input_image5.jpg)
### result_image
![](result_image5.jpg)
### input_image
![](input_image7.jpg)
### result_image
![](result_image7.jpg)
## requirement
opencv
## create an object
TriangleStyle t("xxxxxxxxxx.jpg", high_threshold, low_threshold, random_amount)<br>
"xxxxxxxxxx.jpg" means your picture name<br>
high_threshold and low_threshold control the high threshold and low threshold of Canny operator<br>
random_amount is the number of random points.<br>
## show the result image
call the member function show(), like t.show()
## save the result image
call the member function save(), like t.save()
## detail
[我的博客](www.zerahhah.com/article/12)
