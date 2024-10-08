// entities2 © 2024 by norbcodes is licensed under CC BY-NC 4.0

/**
 * \file datapacks.cpp
 * \author norbcodes
 * \brief Datapacks!! :3
 * \copyright entities2 © 2024 by norbcodes is licensed under CC BY-NC 4.0
 * \details Datapack reading and loading code.
 */

#include <sys/stat.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <pugixml.hpp>
#include <vector>

#include "exit_msg.hpp"
#include "datapacks.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

// Datapack methods

/**
 * \brief Constructor for Datapack class.
 * \param[in] path Path to the .xml file, in <b>const char*</b> form.
 */
Datapack::Datapack(const char* path)
{
    // IN CONSTRUCTOR, ONLY READ "META" SECTION.
    this->Xml.load_file(path);
    
    // Read
    this->Name = this->Xml.child("Datapack").child("Meta").child("Name").text().as_string();
    this->Author = this->Xml.child("Datapack").child("Meta").child("Author").text().as_string();
    this->Description = this->Xml.child("Datapack").child("Meta").child("Description").text().as_string();
}

/**
 * \brief Constructor for Datapack class.
 * \param[in] path Path to the .xml file, in <b>const std::string&</b> form.
 */
Datapack::Datapack(const std::string& path)
{
    // IN CONSTRUCTOR, ONLY READ "META" SECTION.
    this->Xml.load_file(path.c_str());
    
    // Read
    this->Name = this->Xml.child("Datapack").child("Meta").child("Name").text().as_string();
    this->Author = this->Xml.child("Datapack").child("Meta").child("Author").text().as_string();
    this->Description = this->Xml.child("Datapack").child("Meta").child("Description").text().as_string();
}

/**
 * \brief Get Datapack Name.
 * \return The name.
 */
const std::string& Datapack::GetName() const
{
    return (this->Name);
}

/**
 * \brief Get Datapack Author.
 * \return The author.
 */
const std::string& Datapack::GetAuthor() const
{
    return (this->Author);
}

/**
 * \brief Get Datapack Description.
 * \return The description.
 */
const std::string& Datapack::GetDesc() const
{
    return (this->Description);
}

/**
 * \brief Get Datapack XML object.
 * \return The pugi::xml_document object.
 */
const pugi::xml_document& Datapack::GetXml() const
{
    return this->Xml;
}

/**
 * \brief Get Datapack contents and load self.
 */
void Datapack::Load()
{
    // YAAAY now we load :3

    for (pugi::xml_node Data : this->Xml.child("Datapack").child("Data").children())
    {
        if (std::string(Data.name()) == std::string("ExitMsg"))
        {
            // Oki
            // Try to get the "formatted" attribute:
            if (Data.attribute("formatted").as_bool())
            {
                // Formatting ON, format string and put into ExitMsg array
                AddExitMsg(ExitMsgFormatter(Data.text().as_string()));
            }
            else
            {
                // Same thing, but with no formatting
                AddExitMsg(Data.text().as_string());
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

// DatapackEngine methods

/**
 * \brief DatapackEngine constructor.
 * \details Creates a folder for Datapacks ("./datapacks/"), also creates a readme.txt<br>
 *          If folder exists, then it reads all datapacks, and prepares them for loading stage.
 */
DatapackEngine::DatapackEngine()
{
    // Check if "datapacks" folder exists
    struct stat sb;
    if (!stat("./datapacks/", &sb) == 0)
    {
        std::filesystem::create_directory("./datapacks/");
        // Write a lil readme.txt :)
        std::ofstream readme("./datapacks/readme.txt");
        readme << "Hello!!!\nHere, you can put your .xml datapacks for entities2!\nYou may also create subfolders to organize your files.";
        readme.close();
        return;
        // Since we just created the folder, there's obviously not gonna be any XML files yet.
    }
    else
    {
        // Folder DOES exist
        // Now we can actually scan the folder and find datapacks

        // First, reserve our little vector :3
        this->datapacks.reserve(32);

        for (const std::filesystem::directory_entry& file : std::filesystem::recursive_directory_iterator("./datapacks/"))
        {
            // We don't need symlinks support lol
            // This iterates through all stuff in the folder.
            // Check if file:
            if (file.is_regular_file())
            {
                // Check extention
                if (file.path().extension() == ".xml")
                {
                    // :3
                    this->datapacks.emplace_back(file.path().string());
                }
            }
        }
    }
}

/**
 * \brief Load all Datapacks :)
 */
void DatapackEngine::LoadAll()
{
    for (uint8_t i = 0; i != this->datapacks.size(); i++)
    {
        this->datapacks[i].Load();
    }
}

/**
 * \brief Get the amount of Datapacks.
 * \return The amount of Datapacks.
 */
uint32_t DatapackEngine::DatapackCount() const
{
    return this->datapacks.size();
}

/**
 * \brief Return a constant reference to a Datapack instance.
 * \param[in] i Index in the Datapack array.
 * \return Constant reference to a Datapack instance.
 */
const Datapack& DatapackEngine::GetConstDatapackRef(uint32_t i) const
{
    return this->datapacks[i];
}

// entities2 © 2024 by norbcodes is licensed under CC BY-NC 4.0