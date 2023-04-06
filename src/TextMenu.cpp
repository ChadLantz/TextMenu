#include "TextMenu.h"


/**
 * @brief Default constructor
 */
TextMenu::TextMenu()
  : m_parent(nullptr),
    m_currentMenu(this),
    m_overlay(nullptr),
    m_currentEndPoint(nullptr),
    m_name(""),
    m_lineHeight(11),
    m_headerHeight(0),
    m_startLine(0)
{
    for(uint8_t i = 0; i < MAX_MENU_ENTRIES; i++){
        m_entryNames[i] = "";
        m_TextMenuArray[i] = nullptr;
        m_funcArray[i] = nullptr;
    }
    for(uint8_t i = 0; i < BTN_COUNT; i++){
        m_ezButtonArray[i] = nullptr;
        m_ownedButtons[i] = false;
    }
}

/**
 * @brief Submenu constructor. Requires a parent to exist
 * @param _name Name of the submenu. Will be displayed for this entry in the parent, 
 * and at the top of the screen when in this new menu
 * @param _parent The parent menu of this sub menu
 */
TextMenu::TextMenu(String _name, TextMenu &_parent)
  : m_parent(&_parent),
    m_currentMenu(&_parent.getCurrentMenu()),
    m_overlay(nullptr),
    m_currentEndPoint(nullptr),
    m_name(_name),
    m_cursorPos(0),
    m_nEntries(0),
    m_lineHeight(11),
    m_headerHeight(_parent.getHeaderHeight()),
    m_startLine(0)
{
    m_ezButtonArray[0] = &m_parent->getButton(BTN_UP);
    m_ezButtonArray[1] = &m_parent->getButton(BTN_DOWN);
    m_ezButtonArray[2] = &m_parent->getButton(BTN_ENTER);
    m_ezButtonArray[3] = &m_parent->getButton(BTN_BACK);
    for(uint8_t i = 0; i < BTN_COUNT; i++){
        m_ownedButtons[i] = false;
    }
    for(uint8_t i = 0; i < MAX_MENU_ENTRIES; i++){
        m_entryNames[i] = "";
        m_TextMenuArray[i] = nullptr;
        m_funcArray[i] = nullptr;
    }
}

/**
 * @brief Main menu constructor
 * @param _name Name of the menu. Generally "Main"
 * @param _Up The up button
 * @param _Down The down button
 * @param _Enter The enter button
 * @param _Back The back button
 */
TextMenu::TextMenu(String _name, ezButton &_Up, ezButton &_Down, ezButton &_Enter, ezButton &_Back)
  : m_parent(nullptr),
    m_currentMenu(this),
    m_overlay(nullptr),
    m_currentEndPoint(nullptr),
    m_name(_name),
    m_cursorPos(0),
    m_nEntries(0),
    m_lineHeight(11),
    m_headerHeight(0),
    m_startLine(0)
{
    m_ezButtonArray[0] = &_Up;
    m_ezButtonArray[1] = &_Down;
    m_ezButtonArray[2] = &_Enter;
    m_ezButtonArray[3] = &_Back;
    for(uint8_t i = 0; i < BTN_COUNT; i++){
        m_ownedButtons[i] = false;
    }
    for(uint8_t i = 0; i < MAX_MENU_ENTRIES; i++){
        m_entryNames[i] = "";
        m_TextMenuArray[i] = nullptr;
        m_funcArray[i] = nullptr;
    }
}


/**
 * @brief Destructor. TextMenu's own their TextMenu children and any buttons created with the addButton method
 */
TextMenu::~TextMenu(){
    //Delete this menu's children
    for(uint8_t i = 0; i < m_nEntries; i++){
        delete m_TextMenuArray[i];
    }
    //Delete any buttons this menu created
    for(uint8_t i = 0; i < BTN_COUNT; i++){
        if(m_ownedButtons[i]){
            delete m_ezButtonArray[i];
        }
    }
}

/**
 * @brief Add a pre-existing sub menu. This can be used to create a menu that has a different name in its parent
 * than what is displayed at the top when in this menu
 * @param _name Name that will be displayed in the parent menu
 * @param _subMenu The sub menu to be added
 */
void TextMenu::addEntry(String _name, TextMenu &_subMenu){
    m_entryNames[m_nEntries] = _name;
    m_TextMenuArray[m_nEntries] = &_subMenu;
    m_nEntries++;
}

/**
 * @brief Add an entry with a user defined endpoint function
 * @param _name Name of this option in the menu
 * @param func The function to be called
 */
void TextMenu::addEntry(String _name, func_ptr_t func ){
    m_entryNames[m_nEntries] = _name;
    m_funcArray[m_nEntries] = func;
    m_nEntries++;
}

/**
 * @brief Add a sub menu to this menu
 * @param _name Name of this new sub menu
 * @return Reference to the newly created sub menu
 */
TextMenu& TextMenu::addSubMenu(String _name){
    //Make it new so it doesn't go out of scope, but dereference it right away
    //Because references are more user friendly
    TextMenu *newEntry = new TextMenu(_name, *this);
    addEntry(_name, *newEntry);
    return *newEntry;
}

/**
 * @brief Add a button with the pin. This TextMenu instance will claim ownership of this button.
 * @param whichOne Button to be created. Options are BTN_UP, BTN_DOWN, BTN_ENTER, and BTN_BACK
 * @param _pin Which GPIO pin this button is on
 * @return A reference to the new button
 */
ezButton& TextMenu::addButton(BUTTON whichOne, uint8_t _pin){
    //The user requested to assign a button whos function doesn't exist.
    //Make the button, but don't assign it to anything
    if(whichOne >= BTN_COUNT) return *(new ezButton(_pin));

    //If a button already exists in this slot and this TextMenu owns it, delete it
    if(m_ezButtonArray[whichOne] && m_ownedButtons[whichOne]) delete m_ezButtonArray[whichOne];

    //This object owns the new button
    m_ownedButtons[whichOne] = true;

    //Make it new so it doesn't go out of scope, but dereference it right away
    //Because references are more user friendly
    ezButton *button = new ezButton(_pin);
    setButton(whichOne, *button);
    return *button;
}

/**
 * @brief Set the requested button slot to the button provided
 * @param whichOne Button to be set. Options are BTN_UP, BTN_DOWN, BTN_ENTER, and BTN_BACK
 * @param button The button to be placed in the slot
 */
void TextMenu::setButton(BUTTON whichOne, ezButton &button){
    //If a button already exists in this slot, delete it
    if(m_ezButtonArray[whichOne] && m_ownedButtons[whichOne]) delete m_ezButtonArray[whichOne];
    m_ezButtonArray[whichOne] = &button;
}

/**
 * @brief Set the parent of this TextMenu and inform the parent that this child is moving in
 * @param _parent Adoptive parent
 */
void TextMenu::setParent(TextMenu &_parent){
    m_parent = &_parent;
    m_parent->addEntry(m_name,*this);
}

/* This may require some abstract thought
 * We set this object's current draw function, as well
 * as pass it to its parent until no parent exists.
 * This way the root TextMenu always knows what to draw
*/


/**
 * @brief This may require some abstract thought.
 * We set this object's current draw function, as well
 * as pass it to its parent until no parent exists.
 * This way the root TextMenu always knows what to draw and we don't
 * have to search down the tree recursively every time loop() is called
 * @param _menu The menu to be drawn
 */
void TextMenu::setCurrentMenu(TextMenu &_menu){
    m_currentMenu = &_menu;
    if(m_parent){
        m_parent->setCurrentMenu(_menu);
    }
}



/**
 * @brief Sets the overlay for this menu and all its children
 * @param func 
 */
void TextMenu::setOverlay(func_ptr_t func){
    m_overlay = func;
    for(int i = 0; i < MAX_MENU_ENTRIES; i++){
        if(m_TextMenuArray[i]){
            m_TextMenuArray[i]->setOverlay(func);
        }
    }
}


/**
 * @brief This is the method that will be called in loop() for the main menu.
 * This simplifies the process for the user
 * @param display The display to be drawn on
 */
void TextMenu::drawCurrentMenu(OLEDDisplay &display){
    m_currentMenu->draw(display);
}


/**
 * @brief This is the main function to be called in loop. It checks for button presses
 * and draws either the menu items, or the user created endpoint function
 * @param display The display to be drawn on
 */
void TextMenu::draw(OLEDDisplay &display){
    
    //If the header height hasn't been set, just use the line height
    uint8_t headerHeight = (m_headerHeight > 0) ? m_headerHeight : m_lineHeight;    
    
    //Get the number of lines available for the menu entries
    uint8_t nLines = (display.getHeight() - headerHeight)/m_lineHeight;

    //Handle button presses to see if we should draw this menu or something else
    display.clear();
    int8_t doDraw = handleButtons(nLines);

    if( doDraw == -1){ 
        if(m_overlay) m_overlay();
        m_currentMenu->draw(display);
        display.display();
        return;
    }else if(doDraw == 1){
        display.display();
        return;
    }

    if(m_overlay) m_overlay();
    //Draw the name of the current menu top and center
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(display.getWidth()/2, 0, m_name);
    
    //Draw each menu option that will fit on the screen
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    for(uint8_t i = m_startLine; i < min((uint8_t)(m_startLine+nLines),m_nEntries); i++ ){
        uint8_t y = headerHeight + (i-m_startLine)*m_lineHeight;
        display.drawString(10, y, m_entryNames[i]);
        if(i==m_cursorPos){
            display.drawHorizontalLine(2,y+m_lineHeight/2,6);
        }
    }
    display.display();
}

/**
 * @brief Handles button presses and sets internal values accordingly
 * @param nLines Number of lines that can be drawn
 * @return -1 if the current menu has changed and the new one should be drawn
 *          0 if this menu should be drawn 
 *          1 if the endpoint was drawn
 */
int8_t TextMenu::handleButtons(uint8_t _nLines){
    //Update our buttons right away
    for(uint8_t i = 0; i < BTN_COUNT; i++){
        m_ezButtonArray[i]->loop();
    }
    
    //Handle each button press in a chain. This way we can't have simultaneous button presses
    //We are also looking for reasons to return false (don't draw this menu). If we can't find any,
    //return true (draw this menu)
    if(m_currentEndPoint){
        //Call the endpoint function. If it returns false
        //nullify the current endpoint and proceed to draw this menu again
        if(!m_currentEndPoint()){
            m_currentEndPoint = nullptr;
            return 0;
        }else{
            return 1;
        }

    }else if(m_ezButtonArray[BTN_BACK]->isPressed()){
        //We are in a menu and want to go up one level
        //Set the current menu to the parent and draw it
        //If there is no parent, we can't go back so keep drawing this menu
        if(m_parent){ 
            m_startLine = m_cursorPos = 0;
            setCurrentMenu(*m_parent);
            return -1;
        }else{
            return 0;
        }
    }else if(m_ezButtonArray[BTN_UP]->isPressed() && m_cursorPos > 0){
        //Decrement the cursor if we're not already at the top
        m_cursorPos--;
        if(m_cursorPos < m_startLine) m_startLine--;
        return 0;

    }else if(m_ezButtonArray[BTN_DOWN]->isPressed() && m_cursorPos < m_nEntries-1){
        //Increment the cursor if we're not already at the bottom
        m_cursorPos++;
        if(m_cursorPos+1 > m_startLine + _nLines) m_startLine++;
        return 0;

    }else if(m_ezButtonArray[BTN_ENTER]->isPressed()){
        if(m_funcArray[m_cursorPos]){
            //if a function exists, set it as the current endpoint and call it
            m_currentEndPoint = m_funcArray[m_cursorPos];

            //If that function returns false (only wants to be called once)
            //nullify the current endpoint and proceed to draw this menu again
            if(!m_currentEndPoint()){
                m_currentEndPoint = nullptr;
                return 0;
            }else{
                return 1;
            }
        }else if(m_TextMenuArray[m_cursorPos]){
            //if we are on a submenu entry, set it as the current menu and
            //draw that one instead
            setCurrentMenu(*m_TextMenuArray[m_cursorPos]);
            return -1;
        }
    }

    //Catch any cases that weren't covered. (i.e. no buttons pressed)
    return 0;
}
