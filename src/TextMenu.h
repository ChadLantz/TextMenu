#ifndef TextMenu_h
#define TextMenu_h

#if !defined(ARDUINO_ARCH_ESP32) && !defined(ARDUINO_ARCH_ESP8266)
  #error This library only supports ESP32 and ESP8266 boards
#endif

#include <Arduino.h>

#include "OLEDDisplay.h" 

#include <ezButton.h>


enum BUTTON{BTN_UP=0,BTN_DOWN=1,BTN_ENTER=2,BTN_BACK=3,BTN_COUNT=4};
const int MAX_MENU_ENTRIES = 10;

typedef bool (*func_ptr_t)( );

class TextMenu{
  public:

    TextMenu();
    TextMenu(String _name, TextMenu &_parent);
    TextMenu(String _name, ezButton &_Up, ezButton &_Down, ezButton &_Enter, ezButton &_Back);
    
    ~TextMenu();

    void addEntry(String _name, TextMenu &_subMenu);
    void addEntry(String _name, func_ptr_t func );
    TextMenu& addSubMenu(String _name);
    ezButton& addButton(BUTTON whichOne, uint8_t _pin);

    void setButton(BUTTON whichOne, ezButton &button);
    void setParent(TextMenu &_parent);
    void setCurrentMenu(TextMenu &_menu);
    void setOverlay(func_ptr_t func);

    /**
     * @brief Set the line height in number of pixels. Does not change the font, just the distance from line to line
     * @param _height Line height in number of pixels
     */
    void setLineHeight(uint8_t _height){m_lineHeight = _height;}

    void setHeaderHeight(uint8_t _height){m_headerHeight = _height;}

    ezButton& getButton(BUTTON whichOne){return *m_ezButtonArray[whichOne];};
    TextMenu& getParent(){return *m_parent;}
    TextMenu& getCurrentMenu(){return *m_currentMenu;}
    uint8_t getHeaderHeight(){return m_headerHeight;}

    void drawCurrentMenu(OLEDDisplay &display);
    void draw(OLEDDisplay &display);

  private:
    int8_t handleButtons(uint8_t _nLines);
    TextMenu *m_parent;
    ezButton *m_ezButtonArray[BTN_COUNT];
    bool m_ownedButtons[BTN_COUNT];
    TextMenu *m_currentMenu;

    func_ptr_t m_overlay;
    func_ptr_t m_currentEndPoint;
    String m_name;
    String m_entryNames[MAX_MENU_ENTRIES];
    func_ptr_t m_funcArray[MAX_MENU_ENTRIES];
    TextMenu *m_TextMenuArray[MAX_MENU_ENTRIES];
    uint8_t m_cursorPos;
    uint8_t m_nEntries;
    uint8_t m_lineHeight;
    uint8_t m_headerHeight;
    uint8_t m_startLine;
};

#endif