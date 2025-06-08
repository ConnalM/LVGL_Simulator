#pragma once
#include "DisplayModule.h"
#include "ESP32_8048S070_Lvgl_DisplayDriver.h"
#include "SerialDisplay.h"
// #include "WebDisplay.h"

/**
 * @brief Factory for creating display instances
 * 
 * This class is responsible for creating and managing display instances.
 */
class DisplayFactory {
public:
    /**
     * @brief Get the singleton instance
     * 
     * @return DisplayFactory& The singleton instance
     */
    static DisplayFactory& getInstance();
    
    /**
     * @brief Create a display instance
     * 
     * @param type The display type to create
     * @return IBaseDisplay* Pointer to the created display
     */
    IBaseDisplay* createDisplay(DisplayType type);
    
    /**
     * @brief Get a display instance
     * 
     * @param type The display type to get
     * @return IBaseDisplay* Pointer to the display
     */
    IBaseDisplay* getDisplay(DisplayType type);
    
    /**
     * @brief Create a graphical display instance
     * 
     * @param type The display type to create
     * @return IGraphicalDisplay* Pointer to the created graphical display, or nullptr if the display type is not graphical
     */
    IGraphicalDisplay* createGraphicalDisplay(DisplayType type);
    
    /**
     * @brief Get a graphical display instance
     * 
     * @param type The display type to get
     * @return IGraphicalDisplay* Pointer to the graphical display, or nullptr if the display type is not graphical
     */
    IGraphicalDisplay* getGraphicalDisplay(DisplayType type);

private:
    DisplayFactory(); // Private constructor for singleton
    ~DisplayFactory(); // Private destructor
    DisplayFactory(const DisplayFactory&) = delete; // Prevent copy construction
    DisplayFactory& operator=(const DisplayFactory&) = delete; // Prevent assignment

    // Static instance pointer
    static DisplayFactory* _instance;

    // Static cleanup method
    static void destroyInstance();

    // Pointers to managed display instances
    IBaseDisplay* _serialDisplay = nullptr;
    IGraphicalDisplay* _lcdDisplay = nullptr;
    // IBaseDisplay* _webDisplay = nullptr;
};
