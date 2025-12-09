cmake_minimum_required(VERSION 3.26)
# Configurando o padrão c++
set(CMAKE_CXX_STANDARD 17)

# Configurando o nome e executável do projeto
set(PROJECT_NAME "contra-diction")
project(${PROJECT_NAME})

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(OpenGL REQUIRED)

find_package(GLEW QUIET)

if (NOT TARGET GLEW::GLEW)

    find_path(GLEW_INCLUDE_DIR REQUIRED
            NAMES GL/glew.h
            HINTS "C:/Arquivos de Programas/GLEW/include"
    )

    find_library(GLEW_LIBRARY REQUIRED
            NAMES glew32
            HINTS "C:/Arquivos de Programas/GLEW/lib/Release/x64"
    )

    add_library(GLEW::GLEW UNKNOWN IMPORTED)
    set_target_properties(GLEW::GLEW PROPERTIES
            IMPORTED_LOCATION "${GLEW_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${GLEW_INCLUDE_DIR}"
    )

endif()

add_executable(${PROJECT_NAME}
        Source/Renderer/Shader.cpp
        Source/Renderer/Shader.h
        Source/Main.cpp
        Source/Math.cpp
        Source/Math.h
        Source/Random.cpp
        Source/Random.h
        Source/Game.cpp
        Source/Game.h
        Source/AudioSystem.cpp
        Source/AudioSystem.h
        Source/Actors/Actor.cpp
        Source/Actors/Actor.h
        Source/Actors/GasCloud.cpp
        Source/Actors/GasCloud.h
        Source/Actors/ParallaxActor.cpp
        Source/Actors/ParallaxActor.h
        Source/Components/Component.cpp
        Source/Components/Component.h
        Source/Components/Drawing/DrawComponent.cpp
        Source/Components/Drawing/DrawComponent.h
        Source/Components/Drawing/TutorialDrawComponent.cpp
        Source/Components/Drawing/TutorialDrawComponent.h
        Source/Components/Physics/RigidBodyComponent.cpp
        Source/Components/Physics/RigidBodyComponent.h
        Source/Components/Physics/AABBColliderComponent.cpp
        Source/Components/Physics/AABBColliderComponent.h
        Source/Components/ParticleSystemComponent.cpp
        Source/Components/ParticleSystemComponent.h
        Source/Renderer/Font.cpp
        Source/Renderer/Font.h
        Source/Renderer/Renderer.cpp
        Source/Renderer/Renderer.h
        Source/Renderer/VertexArray.h
        Source/Renderer/VertexArray.cpp
        Source/Renderer/Texture.cpp
        Source/Renderer/Texture.h
        Source/UI/UIElement.cpp
        Source/UI/UIElement.h
        Source/UI/UIButton.cpp
        Source/UI/UIButton.h
        Source/UI/UIImage.cpp
        Source/UI/UIImage.h
        Source/UI/UIText.cpp
        Source/UI/UIText.h
        Source/UI/UIRect.cpp
        Source/UI/UIRect.h
        Source/UI/HUD.cpp
        Source/UI/HUD.h
        Source/UI/Screens/UIScreen.cpp
        Source/UI/Screens/UIScreen.h
        Source/UI/Screens/MainMenu.cpp
        Source/UI/Screens/MainMenu.h
        Source/UI/Screens/OptionsScreen.cpp
        Source/UI/Screens/OptionsScreen.h
        Source/UI/Screens/GameOver.cpp
        Source/UI/Screens/GameOver.h
        Source/UI/Screens/PauseMenu.cpp
        Source/UI/Screens/PauseMenu.h
        Source/UI/Screens/InventoryScreen.cpp
        Source/UI/Screens/InventoryScreen.h
        Source/UI/Screens/StageSelect.cpp
        Source/UI/Screens/StageSelect.h
        Source/UI/Screens/CutsceneScreen.cpp
        Source/UI/Screens/CutsceneScreen.h
        Source/UI/Screens/LoadingScreen.cpp
        Source/UI/Screens/LoadingScreen.h
        Source/Actors/Block.cpp
        Source/Actors/Block.h
        Source/Actors/Goomba.cpp
        Source/Actors/Goomba.h
        Source/Actors/Spawner.cpp
        Source/Actors/Spawner.h
        Source/Actors/Spaceman.cpp
        Source/Actors/Spaceman.h
        Source/Actors/SpacemanArm.cpp
        Source/Actors/SpacemanArm.h
        Source/Actors/Laser.cpp
        Source/Actors/Laser.h
        Source/Actors/PlayerBullet.cpp
        Source/Actors/PlayerBullet.h
        Source/Actors/Grass.cpp
        Source/Actors/Grass.h
        Source/Actors/Stone.cpp
        Source/Actors/Stone.h
        Source/Actors/Mushroom.cpp
        Source/Actors/Mushroom.h
        Source/Actors/Coin.cpp
        Source/Actors/Coin.h
        Source/Actors/Policeman.cpp
        Source/Actors/Policeman.h
        Source/Actors/PolicemanBullet.cpp
        Source/Actors/PolicemanBullet.h
        Source/Actors/SoldierBullet.cpp
        Source/Actors/SoldierBullet.h
        Source/Actors/AlienKid.cpp
        Source/Actors/AlienKid.h
        Source/Actors/AlienMan.cpp
        Source/Actors/AlienMan.h
        Source/Actors/AlienWoman.cpp
        Source/Actors/AlienWoman.h
        Source/Components/Drawing/AnimatorComponent.cpp
        Source/Components/Drawing/AnimatorComponent.h
        Source/Components/Drawing/RectComponent.cpp
        Source/Components/Drawing/RectComponent.h
        Source/Components/Drawing/SpriteComponent.cpp
        Source/Components/Drawing/SpriteComponent.h
        Assets/Sprites/Cenario/Vegetation.cpp
        Assets/Sprites/Cenario/Vegetation.h
        Source/Actors/Hazard.cpp
        Source/Actors/Hazard.h
        Source/Actors/EnemyLaser.cpp
        Source/Actors/EnemyLaser.h
        Source/Actors/RobotTurret.cpp
        Source/Actors/RobotTurret.h
        Source/Actors/RobotFlyer.cpp
        Source/Actors/RobotFlyer.h
        Source/Actors/Soldier.cpp
        Source/Actors/Soldier.h
        Source/Actors/FinalFlower.cpp
        Source/Actors/FinalFlower.h
        Source/Actors/FlowerBoss.cpp
        Source/Actors/FlowerBoss.h
        Source/Actors/Vine.cpp
        Source/Actors/Vine.h
        Source/Actors/VineWarning.cpp
        Source/Actors/VineWarning.h
        Source/Actors/LevelEndActor.cpp
        Source/Actors/LevelEndActor.h
        Source/Actors/EndPhaseTrigger.cpp
        Source/Actors/EndPhaseTrigger.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        #mingw pra windows, to no linux usando ninja e visual studio
        SDL2::SDL2main
        SDL2::SDL2
        GLEW::GLEW
        SDL2_image::SDL2_image
        SDL2_mixer::SDL2_mixer
        SDL2_ttf::SDL2_ttf
        OpenGL::GL
)