# octet

This repository was forked from andy-thomason/octet and I hacked it for an assignment for MA Computer Games Art and Design in Autumn term in 2015. This document will include how I changed the codes in Octet and what I learned from this.
I'll firstly show demo play and then explain about it following the instruction that Andy mentioned in the website (http://andy-thomason.github.io/lecture_notes/assignments_2015-2016.html)

###1.Demo in Youtube

I made many mechanics and you can check them with the demo movie.
- Added an artwork of dot-picture in the title screen instead of showing the game title of "invaderes".
- Added the screen of title and game complete
- Also added game statement. It has 3 stages and the player can play the game again and again.
- When the ship collides any invederes, in will die
- In order to notice the player that something happen such as they pushed a key and it works, I added some SEs
- The ship can move not only right and left, but also up and down
- When the missile hits a inveder, it explodes with 8 frame animation
- Added back ground animation of stars to make perspective and fun feeling
- Changed the formation and appearance of the invaderes 
- Change score system. When the ship fires, the missile beats the invader, and the player clears each stage, the score will be increased

###2. Writing your own fragment shader
I added one file for fragment shader which is yuka_shader.h I extracted some codes from the texture_shader.h that had already exsisted. This is simple gradation and is shown the basic background during the game. 

[basic background](https://www.dropbox.com/s/sch73tr90d5iz82/5.png?dl=0) 

###3. Well-formatted code with good layout.
Please look the codes.

###4. Setting up objects by reading CSV file or similar text file.  
(a) Setting up the formation of invaderers
I made 3 CSV files in which 1 and 0 are written that include the positions of the invederers. This game has 3 stages and in each stage it reads different CSV files.

[stage1](https://www.dropbox.com/s/m7p02k32j6l8icg/2.png?dl=0)  
[stage2](https://www.dropbox.com/s/abt2swlreiwvc89/3.png?dl=0)  
[stage3](https://www.dropbox.com/s/w9xpp7z8iifdqo8/4.png?dl=0)  


(b) Setting up an artwork in the title screen  
After learning how to read CSV file and making a fragment shader file, I decided to combine them and create something new that can show my originarity. Then, I tryed making a big dot-picture of a invader and put it into the title screen.

[title screen](https://www.dropbox.com/s/et0fcs8gnyv5we9/1.png?dl=0)

I prepared a CSV file with 1 & 0 written that shows an appearance of an invader by dots. The program reads the CSV file and render squares with my file for fragment shader. Because it renders everyframe with different values for color, the picture's color changes every frame.

###5. Algorithms and problem solving
  (a)Making animation of stars   
    I placed small stars that are far from the ship, middle stars, and big stars that are near from the ship as objects close to the subject should be big in the view of perspective. Then, set the speed respectively. Far stars should move slower, and vice varsa in perspective. Also, I set the place of the stars randomly. When the stars get out of the screen, they go back to random place above the screen and appear again. I did it because if a same background picture comes again and again the players notice the algorithms easily and that makes them bored.   
    
  (b)Making animation of explosion  
  In order to emphasis the fun experience that is player succeded in beating an inveder, the animation of explosion(and SE) would be very effective. The Algorithms are the following. When the missile hit the enemy, 8 gif pictures come to the same place as the invader killed and change their status to "true ". The order of the picture is that earlier stage of the explosion is closer to the top. At the same time, there is function that is always searching for explosion sprite of the first stage whose status is "true". If it finds it, it remove the sprite out of the screen and the rest of 7 sprites 1 by 1.  Consequently, 8 frame animation show up on the screen.
  
  (c)Learning C++  
  Let me note that I didn't know anything about C++ and didn't know many math terms in English, so I began reading a book for C++ beginner and got some idea. It was the biggest problem solving for me, but I know I'm still very beginner who needs learning more.   
     
 (d) About the Key command  
 I found that "is_key_going_up" works the same as "is_key_going_down". Then I checked app_common.h and there was codes like the following

    /// returns true if a key has gone down this frame
    bool is_key_going_down(unsigned key) {
      return keys[key & 0xff] != 0 && prev_keys[key & 0xff] == 0;
    }

    /// returns true if a key has gone down this frame
    bool is_key_going_up(unsigned key) {
      return keys[key & 0xff] != 0 && prev_keys[key & 0xff] == 0;
    }
  
  
  These two are exactly the same except the name of the bool, but  should be different more. However, unfortunately it's beyond my knowledge to fix it. I just note that when I changed the last value of zero to one, it seemed to have fixed.
 
 ###6. Summery   
    To sum up, I changed the simple shooting game into multi-stage game which can provide player with fun by scoring system, SE, and artwork by using basical C++, fragment shader, and CSV file.  Now I can imagine how they made great games like 沙羅曼蛇 <https://www.youtube.com/watch?v=o8nv51f4vh0> and Hector87 <https://www.youtube.com/watch?v=23w9-WNq_1Q> .　By learning these games, I would be able to add life gauge, invaderes that move differently and come from above the screen, add more fascinating vidual effect, and so on. Additionally, I thought I can generate bgm using CSV file as a music score with informaion of pitch and duration if I knew how to generate simple sine wave using C++. These ideas will be put off for next challenge of learning.
