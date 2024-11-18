## How To Install RaspberrryPiPico
# How To Install CMake and GCC Complier for Linux
```
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
```
# How To Install CMake for Window 10/11
- Go to the [website](https://cmake.org/download/) and download the latest version.
- Double click to run the installer. ![Installer image](https://cdn.discordapp.com/attachments/1013620162471141476/1308121567259131997/image.png?ex=673ccaa5&is=673b7925&hm=a4aa51c611ebc6d7bf90b09003cd5f56cd879bae0d6d981019f49b8c44fbe8d0&)
- When asked for selecting option, select “Add CMake to the system PATH for all users”. Then click install and finish button. ![Option](https://perso.uclouvain.be/allan.barrea/opencv/_images/cmake_add_path.png)
- Add the path to the environment variables. You have to open “Edit the system environment variables”. ![Environment Variable](https://files.codingninjas.in/article_images/custom-upload-1683458579.webp)
- Click “Environment Variables”. ![Button](https://files.codingninjas.in/article_images/custom-upload-1683458591.webp)
- Go to “System Variables” and click on “path”. ![Path](https://files.codingninjas.in/article_images/custom-upload-1683458604.webp)
- Add the path of the CMake folder along with its bin file. ![Add path](https://cdn.discordapp.com/attachments/1196726335867998269/1308132320414273628/image.png?ex=673cd4a9&is=673b8329&hm=15514336b76a615eafdb498f2a0c7ea687a2e449b91af005be9ab05bcf4717f2&)
- To check installation, open the command prompt and type "cmake", and it should show below picture: ![CMake Installtion](https://cdn.discordapp.com/attachments/1013620162471141476/1308125171240009841/image.png?ex=673cce00&is=673b7c80&hm=fb2156c725cb1eb3142b0aadb57e08db812d833054b40f854576f9aedbbb1569&)
# How To Install GCC Compiler for Windows 10/11
- Open the [link](https://sourceforge.net/projects/mingw-w64/) and click the “Download” button to download the MinGW-w64 - for 32 and 64 bit Windows. ![download](https://files.codingninjas.in/article_images/custom-upload-1683458386.webp)
- After downloading, you must extract the zip file by right click and choose extract all option and install the folder and “mingw-get-setup” application. ![zip folder](https://files.codingninjas.in/article_images/custom-upload-1683458487.webp)
- Click Install Button. ![Install button](https://files.codingninjas.in/article_images/custom-upload-1683517444.png)
- It will appear Installation preferences. Keep this as default and don’t change anything. Click on the “Continue” button and it will move to the next page. ![Installation preferences](https://files.codingninjas.in/article_images/custom-upload-1683458514.webp)
- Wait downloading the “MinGW Installation Manager” as shown in the image below. ![downloading manager](https://files.codingninjas.in/article_images/custom-upload-1683458527.webp)
- It is mandatory to mark “mingw-developer-tool” (provide some necessary developer tools) and “mingw32-base” (basic MinGW installation). As the image below, there are multiple versions of GNU Compilers. It depends on your needs, and which compiler you want to install. ![Compilers](https://files.codingninjas.in/article_images/custom-upload-1683458541.webp)
- Click on the “Apply Changes” to install all the libraries, header-files and modules of the GNU Compiler that selected. ![Apply Change](https://files.codingninjas.in/article_images/custom-upload-1683458554.webp)
- Add the path to the environment variables. You have to open “Edit the system environment variables”. ![Environment Variable](https://files.codingninjas.in/article_images/custom-upload-1683458579.webp)
- Click “Environment Variables”. ![Button](https://files.codingninjas.in/article_images/custom-upload-1683458591.webp)
- Go to “System Variables” and click on “path”. ![Path](https://files.codingninjas.in/article_images/custom-upload-1683458604.webp)
- Add the path of the MinGW folder along with its bin file. ![Add path](https://files.codingninjas.in/article_images/custom-upload-1683458614.webp)
- It is good to go now. Open commander prompt and the image below should show the g++ version installed. ![finish installation](https://files.codingninjas.in/article_images/custom-upload-1683458629.webp) 