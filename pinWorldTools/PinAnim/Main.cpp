/// TODO: SAVE/OPEN FILE DIALOG

#define BG_COLOR sf::Color(50, 50, 100)

#define FMT_ANIM_FILE "/PINANIM\nRECT= %d, %d, %d, %d;\nFR_AMOUNT= %d;\nINTERVAL= %f;\nPINANIM/"
#define FMT_HEADER_FILE "Animation %s; /// for %s"
#define FMT_SRC_FILE "%s = Animation(%d, sf::IntRect(%d, %d, %d, %d), sf::seconds(%f));"

#define MAX_SAVEPATH_SIZE 16

#include "imgui.h"
#include "imgui-SFML.h"

#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdio>
#include <filesystem>
#include <string>


bool openAnimationFile(std::string), openImageFile(std::string);
const char* DEFAULT_IMG_PATH = "default", *DEFAULT_ANIM_NAME = "", *EXTENSION = ".panim";

void anim2cpp(std::string);

std::filesystem::__cxx11::directory_entry animFolderPath("animation"), imgFolderPath("image"), srcFolderPath("anim_cpp"), incFolderPath("anim"),
                        implFolderPath("animImpl");
std::string* filePaths;
FILE* file_stream;
char* nameOfUsedImage;
char** animationFiles_cStr;

unsigned int countOfFiles = 0;
int selectedFile = 0;

bool bProcessAnim = false, sel = false;

void processAnimation();
void initAnimation(sf::IntRect, unsigned int, sf::Time);
void saveFile();

sf::RenderWindow mainWin;

sf::Sprite preview;
sf::Texture prTexture;

sf::IntRect actualAnimationRect;
unsigned int actualFramesAmount;
int firstRectPos;
sf::Time actualTimeInterval, actualTime;

sf::IntRect animationRect;
int framesAmount = 2;
short int dialogMode = 0;
float timeInterval = 1.f;

sf::Clock antimer, imdt;

void procImgFiles();
void procAnimFiles();

bool createAnimFolder();
bool createImgFolder();
bool createSrcFolder();

char* pathToSave;

int main()
{
    pathToSave = new char[MAX_SAVEPATH_SIZE];
   // nameOfUsedImage = new char[MAX_SAVEPATH_SIZE];

   // strcpy(nameOfUsedImage, DEFAULT_IMG_PATH);
   strcpy(pathToSave, DEFAULT_ANIM_NAME);

    createAnimFolder();
    createImgFolder();
    createSrcFolder();

    animationRect = sf::IntRect(1, 1, 100, 100);

    antimer.restart();
    mainWin.create(sf::VideoMode(800, 600), "PinAnim", sf::Style::Default);

    prTexture.loadFromFile("default.jpg");
    preview.setTexture(prTexture);
    preview.setPosition(100, 200);

    initAnimation(sf::IntRect(0, 0, 100, 100), 3, sf::seconds(1.f));
    ImGui::SFML::Init(mainWin);

    while(mainWin.isOpen())
    {
        actualTime = antimer.getElapsedTime();
        sf::Event ev;
        while(mainWin.pollEvent(ev))
        {
            ImGui::SFML::ProcessEvent(ev);
            if(ev.type == sf::Event::Closed)
                mainWin.close();
        }
        ImGui::SFML::Update(mainWin, imdt.restart());

        if(bProcessAnim)
            processAnimation();

        ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_::ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar |
                     ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar);
            if
            (
                ImGui::InputInt("Rect X", &animationRect.left, 1) ||
                ImGui::InputInt("Rect Y", &animationRect.top, 1) ||
                ImGui::InputInt("Rect Width", &animationRect.width, 1) ||
                ImGui::InputInt("Rect Height", &animationRect.height, 1) ||
                ImGui::InputInt("Frames Amount", &framesAmount, 1) ||
                ImGui::InputFloat("Time Interval", &timeInterval, 0.5f)
            )

            {
                initAnimation(sf::IntRect(animationRect.left, animationRect.top, animationRect.width, animationRect.height), framesAmount, sf::seconds(timeInterval));
            }

            if(ImGui::BeginMenuBar())
            {
                if(ImGui::BeginMenu("File"))
                {
                    if(ImGui::BeginMenu("Open"))
                    {
                        if(ImGui::MenuItem("Image"))
                        {
                            dialogMode = 1;
                            procImgFiles();
                            printf("%d\n", 88);
                        }
                        if(ImGui::MenuItem("Animation"))
                        {
                            dialogMode = 2;
                            procAnimFiles();
                            printf("%d\n", 89);
                        }
                        ImGui::EndMenu();
                    }
                    if(ImGui::MenuItem("Save"))
                    {
                        dialogMode = 4;
                    }
                    if(ImGui::MenuItem("Convert"))
                    {
                        dialogMode = 3;
                        procAnimFiles();
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }


            if(ImGui::Button("Play"))
                bProcessAnim = true;
            if(ImGui::Button("Stop"))
            {
                bProcessAnim = false;
                actualAnimationRect.left = firstRectPos;
                preview.setTextureRect(actualAnimationRect);
            }


       ImGui::End();

       int i = 0;
       if(dialogMode < 4 && dialogMode != 0)
       {
           ImGui::Begin("Open file", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_::ImGuiWindowFlags_NoResize);
           if(ImGui::BeginListBox("Files", ImVec2(300, 200)))
           {
              for(int i = 0; i < countOfFiles; i++)
              {
                  if(ImGui::Selectable(filePaths[i].c_str(), false))
                    {
                        if(dialogMode == 1)
                        {
                            openImageFile(filePaths[i]);
                            //strcpy( nameOfUsedImage, std::filesystem::__cxx11::path(filePaths[i].c_str() ).filename().string().c_str() );
                        }

                        else if(dialogMode == 2)
                        {
                            openAnimationFile(filePaths[i]);
                        }
                        else if(dialogMode == 3)
                        {
                            anim2cpp(filePaths[i]);
                        }

                        dialogMode = 0;
                    }
              }

              ImGui::EndListBox();
           }

           if(ImGui::Button("Cancel", ImVec2(50, 30)))
                dialogMode = 0;

           ImGui::End();
       }
       else if(dialogMode == 4)
       {
            ImGui::Begin("Save animation file", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_::ImGuiWindowFlags_NoMove );
                ImGui::InputText("Filename", pathToSave, MAX_SAVEPATH_SIZE);
                if(ImGui::Button("Save", ImVec2(50, 30)))
                {
                    saveFile();
                    dialogMode = 0;
                }
                if(ImGui::Button("Cancel", ImVec2(50, 30)))
                {
                    dialogMode = 0;
                }
            ImGui::End();

            ImGui::SetWindowSize("Save animation file", ImVec2(300, 250));
       }

       ImGui::Begin("Status", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_NoMove | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse);

            ImGui::LabelText("Rect X position", "%d", actualAnimationRect.left);
            ImGui::LabelText("Rect Y position", "%d", actualAnimationRect.top);
            ImGui::LabelText("Rect width", "%d", actualAnimationRect.width);
            ImGui::LabelText("Rect height", "%d", actualAnimationRect.height);
            ImGui::LabelText("Frames amount", "%d", actualFramesAmount);
            if(bProcessAnim)
                ImGui::LabelText("Current time", "%.2f", actualTime.asSeconds());

       ImGui::End();


        ImGui::SetWindowPos("Properties", ImVec2(400, 0), ImGuiCond_::ImGuiCond_Once);
        ImGui::SetWindowSize("Properties", ImVec2(400, 300));

        ImGui::SetWindowPos("Status", ImVec2(400, 300), ImGuiCond_::ImGuiCond_Once);
        ImGui::SetWindowSize("Status", ImVec2(400, 300));

        mainWin.clear(BG_COLOR);
        mainWin.draw(preview);
        ImGui::SFML::Render(mainWin);
        mainWin.display();
    }
    ImGui::SFML::Shutdown();
}


void processAnimation()
{
    if(actualFramesAmount > 0)
    {

        if(antimer.getElapsedTime() >= actualTimeInterval)
        {
            actualAnimationRect.left += actualAnimationRect.width;
            preview.setTextureRect(actualAnimationRect);

            antimer.restart();
        }
        if(actualAnimationRect.left >= actualAnimationRect.width * actualFramesAmount)
        {
            actualAnimationRect.left = firstRectPos;
            preview.setTextureRect(actualAnimationRect);

            antimer.restart();
        }

    }
}

void initAnimation(sf::IntRect rect, unsigned int amount, sf::Time interval)
{
    preview.setTextureRect(rect);
    actualAnimationRect = rect;
    actualFramesAmount = amount;
    firstRectPos = rect.left;
    actualTimeInterval = interval;

    antimer.restart();
}

void procAnimFiles()
{
    countOfFiles = 0;
    for(auto path: std::filesystem::__cxx11::directory_iterator(animFolderPath))
        countOfFiles++;

    filePaths = new std::string[countOfFiles];

    //std::cout << "COunt of files is: " << countOfFiles << std::endl;

    int i = 0;

    for(auto path: std::filesystem::__cxx11::directory_iterator(animFolderPath))
    {
        filePaths[i] = path.path().string();
        i++;
    }
}

void procImgFiles()
{
    countOfFiles = 0;
    for(auto path: std::filesystem::__cxx11::directory_iterator(imgFolderPath))
        countOfFiles++;

    filePaths = new std::string[countOfFiles];

    //std::cout << "COunt of files is: " << countOfFiles << std::endl;

    int i = 0;

    for(auto path: std::filesystem::__cxx11::directory_iterator(imgFolderPath))
    {
        filePaths[i] = path.path().string();
        i++;
    }
}
bool createAnimFolder()
{
    if( !( std::filesystem::exists(animFolderPath.path()) ) )
    {
        std::filesystem::create_directory(animFolderPath.path());
        return false;
    }
    else
        return true;
}

bool createImgFolder()
{
    if( !( std::filesystem::exists(imgFolderPath.path()) ) )
    {
        std::filesystem::create_directory(imgFolderPath.path());
        return false;
    }
    else
        return true;
}
bool openAnimationFile(std::string path)
{
    std::cout << "Trying to open\n";
    file_stream = fopen(path.c_str(), "r");
    fscanf(file_stream, FMT_ANIM_FILE,
           &animationRect.left,
           &animationRect.top, &animationRect.width,
           &animationRect.height,
           &framesAmount,
           &timeInterval);

           initAnimation(sf::IntRect(animationRect.left, animationRect.top, animationRect.width, animationRect.height), framesAmount, sf::seconds(framesAmount));
           fclose(file_stream);

}

void saveFile()
{
    strcat(pathToSave, EXTENSION);
    std::filesystem::__cxx11::path final_path = animFolderPath/(std::filesystem::__cxx11::path(pathToSave));
    std::cout << final_path.string() << std::endl;
    file_stream = fopen(final_path.string().c_str(), "w");

    fprintf(file_stream, FMT_ANIM_FILE,
            animationRect.left,
            animationRect.top,
            animationRect.width,
            animationRect.height,
            framesAmount,
            timeInterval);

    fclose(file_stream);
}
bool openImageFile(std::string path)
{
    if(prTexture.loadFromFile(path))
        return true;
    else
        return false;
}

void anim2cpp(std::string pathToAnim)
{
    sf::IntRect rect;
    unsigned int amount;
    float interval;
    char* imgName = new char[MAX_SAVEPATH_SIZE];

    file_stream = fopen(pathToAnim.c_str(), "r");

    fscanf(file_stream, FMT_ANIM_FILE,
           &rect.left,
           &rect.top, &rect.width,
           &rect.height,
           &amount,
           &interval);

    fclose(file_stream);
    std::filesystem::__cxx11::path final_path = srcFolderPath.path()/incFolderPath.path();
    std::string final_path_str = final_path.string() + "\\" + std::filesystem::__cxx11::path(pathToAnim).stem().string() + ".hpp";
    std::cout << final_path_str << std::endl;

    file_stream = fopen(final_path_str.c_str(), "w");
    fprintf(file_stream, FMT_HEADER_FILE, std::filesystem::__cxx11::path(pathToAnim).stem().string().c_str(), imgName);
    fclose(file_stream);

    final_path = srcFolderPath.path()/implFolderPath.path();
    final_path_str = final_path.string() + "\\" + std::filesystem::__cxx11::path(pathToAnim).stem().string() + ".cpp";
    std::cout << final_path_str << std::endl;

    file_stream = fopen(final_path_str.c_str(), "w");
    fprintf(file_stream, FMT_SRC_FILE, std::filesystem::__cxx11::path(pathToAnim).stem().string().c_str(),
            amount, rect.left,
            rect.top,
            rect.width,
            rect.height,
            interval);
    fclose(file_stream);
}

bool createSrcFolder()
{
    if( !( std::filesystem::exists(srcFolderPath.path()) ) )
    {
        std::filesystem::create_directory(srcFolderPath.path());
    }
    if( !( std::filesystem::exists(srcFolderPath.path()/incFolderPath.path()) ) )
    {
        std::filesystem::create_directory(srcFolderPath.path()/incFolderPath.path());
    }
    if( !( std::filesystem::exists(srcFolderPath.path()/implFolderPath.path()) ) )
    {
        std::filesystem::create_directory(srcFolderPath.path()/implFolderPath.path());
    }

    return true;
}

