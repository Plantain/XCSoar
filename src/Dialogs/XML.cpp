/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009

	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>
	Max Kellermann <max@duempel.org>
	Tobias Bieniek <tobias.bieniek@gmx.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "Dialogs/XML.hpp"
#include "Dialogs/Message.hpp"
#include "Language.hpp"
#include "xmlParser.h"
#include "LocalPath.hpp"
#include "DataField/Boolean.hpp"
#include "DataField/Enum.hpp"
#include "DataField/FileReader.hpp"
#include "DataField/Float.hpp"
#include "DataField/Integer.hpp"
#include "DataField/String.hpp"
#include "UtilsFile.hpp"
#include "Screen/Fonts.hpp"
#include "Screen/Layout.hpp"
#include "Screen/SingleWindow.hpp"
#include "Interface.hpp"
#include "Form/Form.hpp"
#include "Form/Frame.hpp"
#include "Form/Edit.hpp"
#include "Form/EventButton.hpp"
#include "Form/SymbolButton.hpp"
#include "Form/Draw.hpp"
#include "Form/List.hpp"
#include "Form/Tabbed.hpp"
#include "Form/Panel.hpp"
#include "Form/Keyboard.hpp"
#include "StringUtil.hpp"

#include <stdio.h>    // for _stprintf
#include <tchar.h>
#include <limits.h>

#include <algorithm>

// used when stretching dialog and components
double g_ddlgScaleWidth = 1.0;
// to full width of screen
DialogStyle_t g_eDialogStyle = eDialogFullWidth;

using std::min;

/**
 * Converts a String into an Integer and returns
 * the default value if String = NULL
 * @param String The String to parse
 * @param Default The default return value
 * @return The parsed Integer value
 */
static long
StringToIntDflt(const TCHAR *String, long Default)
{
  if (String == NULL || string_is_empty(String))
    return Default;
  return _tcstol(String, NULL, 0);
}

/**
 * Converts a String into a Float and returns
 * the default value if String = NULL
 * @param String The String to parse
 * @param Default The default return value
 * @return The parsed Float value
 */
static double
StringToFloatDflt(const TCHAR *String, double Default)
{
  if (String == NULL || string_is_empty(String))
    return Default;
  return _tcstod(String, NULL);
}

/**
 * Returns the default value if String = NULL
 * @param String The String to parse
 * @param Default The default return value
 * @return The output String
 */
static const TCHAR *
StringToStringDflt(const TCHAR *String, const TCHAR *Default)
{
  if (String == NULL || string_is_empty(String))
    return Default;
  return String;
}

/**
 * Converts a String into a Color and sets
 * a default value if String = NULL
 * @param String The String to parse
 * @param color The color (output)
 */
static bool
StringToColor(const TCHAR *String, Color &color)
{
  long value = StringToIntDflt(String, -1);
  if (value & ~0xffffff)
    return false;

  color = Color((value >> 16) & 0xff, (value >> 8) & 0xff, value & 0xff);
  return true;
}

/**
 * Returns the dialog style property ("Popup") of the given node
 * @param xNode The node to check
 * @return Dialog style (DialogStyle_t), Default = FullWidth
 */
static DialogStyle_t
GetDialogStyle(XMLNode *xNode) 
{
  const TCHAR* popup = xNode->getAttribute(_T("Popup"));
  if ((popup == NULL) || string_is_empty(popup))
    return g_eDialogStyle;
  else
    return (DialogStyle_t)StringToIntDflt(popup, 0);
}

static int 
Scale_Dlg_Width(const int x, const DialogStyle_t eDialogStyle) 
{
  if (!Layout::ScaleSupported())
    // todo: return x; ?!
    return Layout::Scale(x);

  if (eDialogStyle == eDialogFullWidth)
    // stretch width to fill screen horizontally
    return (int)(x * g_ddlgScaleWidth);
  else
    return Layout::Scale(x);
}

/**
 * This function reads the following parameters from the XML Node and
 * saves them as Control properties:
 * Name, x-, y-Coordinate, Width, Height, Font and Caption
 * @param Node The XML Node that represents the Control
 * @param Name Name of the Control (pointer)
 * @param X x-Coordinate of the Control (pointer)
 * @param Y y-Coordinate of the Control (pointer)
 * @param Width Width of the Control (pointer)
 * @param Height Height of the Control (pointer)
 * @param Font Font of the Control (pointer)
 * @param Caption Caption of the Control (pointer)
 * @param eDialogStyle Dialog style of the Form
 */
static void
GetDefaultWindowControlProps(XMLNode *Node, TCHAR *Name, int *X, int *Y,
                             int *Width, int *Height, int *Font,
                             TCHAR *Caption, const DialogStyle_t eDialogStyle)
{
  // Calculate x- and y-Coordinate
  *X = Scale_Dlg_Width(StringToIntDflt(Node->getAttribute(_T("X")), 0),
                       eDialogStyle);
  *Y = StringToIntDflt(Node->getAttribute(_T("Y")), 0);
  if (*Y >= 0) { // not -1
    (*Y) = Layout::Scale(*Y);
  }

  // Calculate width and height
  *Width = Scale_Dlg_Width(StringToIntDflt(Node->getAttribute(_T("Width")), 50),
                           eDialogStyle);
  *Height = StringToIntDflt(Node->getAttribute(_T("Height")), 50);
  if (*Height >= 0) {
    (*Height) = Layout::Scale(*Height);
  }

  // Determine font style
  *Font = StringToIntDflt(Node->getAttribute(_T("Font")), -1);

  // Determine name and caption
  _tcscpy(Name, StringToStringDflt(Node->getAttribute(_T("Name")), _T("")));
  _tcscpy(Caption, StringToStringDflt(Node->getAttribute(_T("Caption")), _T("")));

  // TODO code: Temporary double handling to
  // fix "const unsigned short*" to "unsigned short *" problem

  // Translate caption
  const TCHAR *translated = gettext(Caption);
  if (translated != Caption)
    _tcscpy(Caption, translated);
}

static void *
CallBackLookup(CallBackTableEntry_t *LookUpTable, const TCHAR *Name)
{
  int i;

  if (LookUpTable != NULL && Name != NULL && !string_is_empty(Name))
    for (i = 0; LookUpTable[i].Ptr != NULL; i++) {
      if (_tcscmp(LookUpTable[i].Name, Name) == 0) {
        return LookUpTable[i].Ptr;
      }
    }

  return NULL;
}

static void
LoadChildrenFromXML(WndForm &form, ContainerControl *Parent,
                    CallBackTableEntry_t *LookUpTable,
                    XMLNode *Node, int Font, const DialogStyle_t eDialogStyle);

static Font *FontMap[5] = {
  &TitleWindowFont,
  &MapWindowFont,
  &MapWindowBoldFont,
  &CDIWindowFont,
  &InfoWindowFont
};

#ifdef WIN32

static XMLNode
xmlLoadFromResource(const TCHAR* lpName, XMLResults *pResults)
{
  LPTSTR lpRes;
  HRSRC hResInfo;
  HGLOBAL hRes;
  int l, len;

  // Find the xml resource.
  hResInfo = FindResource(XCSoarInterface::hInst, lpName, _T("XMLDialog"));

  if (hResInfo == NULL) {
    MessageBoxX(gettext(_T("Can't find resource")), gettext(_T("Dialog error")),
                MB_OK | MB_ICONEXCLAMATION);

    // unable to find the resource
    return XMLNode::emptyXMLNode;
  }

  // Load the wave resource.
  hRes = LoadResource(XCSoarInterface::hInst, hResInfo);

  if (hRes == NULL) {
    MessageBoxX(gettext(_T("Can't load resource")), gettext(_T("Dialog error")),
                MB_OK | MB_ICONEXCLAMATION);

    // unable to load the resource
    return XMLNode::emptyXMLNode;
  }

  // Lock the wave resource and do something with it.
  lpRes = (LPTSTR)LockResource(hRes);

  if (lpRes) {
    l = SizeofResource(XCSoarInterface::hInst, hResInfo);
    if (l > 0) {
      char *buf = (char*)malloc(l + 2);
      if (!buf) {
        MessageBoxX(gettext(_T("Can't allocate memory")), gettext(
            _T("Dialog error")), MB_OK | MB_ICONEXCLAMATION);
        // unable to allocate memory
        return XMLNode::emptyXMLNode;
      }
      strncpy(buf, (char*)lpRes, l);
      buf[l] = 0; // need to explicitly null-terminate.
      buf[l + 1] = 0;
      len = l;

#ifdef _UNICODE
      LPTSTR b2 = (LPTSTR)malloc(l * 2 + 2);
      MultiByteToWideChar(CP_ACP,          // code page
                          MB_PRECOMPOSED,  // character-type options
                          buf,             // string to map
                          l,               // number of bytes in string
                          b2,              // wide-character buffer
                          l * 2 + 2);      // size of buffer
      free(buf);
      buf = (char*)b2;
      buf[l * 2] = 0;
      buf[l * 2 + 1] = 0;
#endif

      XMLNode x = XMLNode::parseString((LPTSTR)buf, pResults);

      free(buf);
      return x;
    }
  }

  MessageBoxX(gettext(_T("Can't lock resource")), gettext(_T("Dialog error")),
              MB_OK | MB_ICONEXCLAMATION);

  return XMLNode::emptyXMLNode;
}

/**
 * Tries to load an XML file from the resources
 * @param lpszXML The resource name
 * @param tag (?)
 * @return The parsed XMLNode
 */
static XMLNode
xmlOpenResourceHelper(const TCHAR *lpszXML)
{
  XMLResults pResults;

  pResults.error = eXMLErrorNone;
  XMLNode::GlobalError = false;
  XMLNode xnode = xmlLoadFromResource(lpszXML, &pResults);
  if (pResults.error != eXMLErrorNone) {
    XMLNode::GlobalError = true;
    TCHAR errortext[100];
    _stprintf(errortext,_T("%s %i %i"), XMLNode::getError(pResults.error),
              pResults.nLine, pResults.nColumn);

    MessageBoxX(errortext, gettext(_T("Dialog error")),
                MB_OK | MB_ICONEXCLAMATION);
  }
  return xnode;
}

#endif /* WIN32 */

/**
 * This function searches for the given (file)name and if not found
 * resource and returns the main XMLNode
 * @param name File to search for
 * @param resource Resource to search for
 * @return The main XMLNode
 */
static const XMLNode
load_xml_file_or_resource(const TCHAR *name, const TCHAR* resource)
{
  XMLNode xMainNode;

  // Get filepath
  char FileName[MAX_PATH];
  LocalPath(FileName, name);

  // If file exists -> Load XML from file
  if (FileExists(FileName))
    xMainNode = XMLNode::openFileHelper(FileName);

#ifdef WIN32
  // If XML file hasn't been loaded
  if (xMainNode.isEmpty()) {

    // and resource exists
    if (resource)
      // -> Load XML from resource
      xMainNode = xmlOpenResourceHelper(resource);
  }
#endif

  return xMainNode;
}

/**
 * Loads the color information from the XMLNode and sets the fore- and
 * background color of the given WindowControl
 * @param wc The WindowControl
 * @param node The XMLNode
 */
static void
LoadColors(WindowControl &wc, const XMLNode &node)
{
  Color color;

  if (StringToColor(node.getAttribute(_T("BackColor")), color))
    wc.SetBackColor(color);

  if (StringToColor(node.getAttribute(_T("ForeColor")), color))
    wc.SetForeColor(color);
}

static void
CalcWidthStretch(XMLNode *xNode, const RECT rc, const DialogStyle_t eDialogStyle)
{
  int Width = StringToIntDflt(xNode->getAttribute(_T("Width")), 50);

  if ((eDialogStyle == eDialogFullWidth) && Layout::ScaleSupported())
    g_ddlgScaleWidth = (double)(rc.right - rc.left) / (double)Width;
  else
    g_ddlgScaleWidth = Layout::Scale(1); // retain dialog geometry
}

/**
 * This function returns a WndForm created either from the ressources or
 * from the XML file in XCSoarData(if found)
 * @param LookUpTable The CallBackTable
 * @param FileName The XML filename to search for in XCSoarData
 * @param Parent The parent window (e.g. XCSoarInterface::main_window)
 * @param resource The resource to look for
 * @return The WndForm object
 */
WndForm *
dlgLoadFromXML(CallBackTableEntry_t *LookUpTable, const TCHAR *FileName,
               SingleWindow &Parent, const TCHAR* resource)
{

  WndForm *theForm = NULL;

  // assert(main_window == Parent);  // Airspace warning has MapWindow as parent,
  // ist that ok?  JMW: No, I think that it is better to use main UI thread for
  // everything.  See changes regarding RequestAirspaceDialog in AirspaceWarning.cpp

  // Find XML file or resource and load XML data out of it
  XMLNode xMainNode = load_xml_file_or_resource(FileName, resource);

  // TODO code: put in error checking here and get rid of exits in xmlParser
  // If XML error occurred -> Error messagebox + cancel
  if (xMainNode.isEmpty()) {
    MessageBoxX(gettext(_T("Error in loading XML dialog")),
                gettext(_T("Dialog error")), MB_OK | MB_ICONEXCLAMATION);

    return NULL;
  }

  XMLNode xNode;

  // If the main XMLNode is of type "WndForm"
  if (_tcsicmp(xMainNode.getName(), _T("WndForm")) == 0)
    // -> save it as the dialog node
    xNode = xMainNode;
  else
    // Get the first child node of the type "WndForm"
    // and save it as the dialog node
    xNode = xMainNode.getChildNode(_T("WndForm"));

  FontMap[0] = &TitleWindowFont;
  FontMap[1] = &MapWindowFont;
  FontMap[2] = &MapWindowBoldFont;
  FontMap[3] = &CDIWindowFont;
  FontMap[4] = &InfoWindowFont;

  // If Node does not exists -> Error messagebox + cancel
  if (xNode.isEmpty()) {
    MessageBoxX(gettext(_T("Error in loading XML dialog")),
                gettext(_T("Dialog error")), MB_OK | MB_ICONEXCLAMATION);

    return NULL;
  }

  int X, Y, Width, Height, Font;
  TCHAR sTmp[128];
  TCHAR Name[64];

  // todo: this dialog style stuff seems a little weird...

  // Determine the dialog style of the dialog
  DialogStyle_t eDialogStyle = GetDialogStyle(&xNode);

  // Determine the dialog size
  const RECT rc = Parent.get_client_rect();
  CalcWidthStretch(&xNode, rc, eDialogStyle);

  GetDefaultWindowControlProps(&xNode, Name, &X, &Y, &Width, &Height, &Font,
                               sTmp, eDialogStyle);

  // Correct dialog size and position for dialog style
  switch (eDialogStyle) {
  case eDialogFullWidth:
    X = rc.top;
    Y = rc.bottom;
    Width = rc.right - rc.left; // stretch form to full width of screen
    Height = rc.bottom - rc.top;
    X = 0;
    Y = 0;
    break;
  case eDialogScaled:
    break;
  case eDialogScaledCentered:
    X = (rc.right - rc.left) / 2; // center form horizontally on screen
    break;
  case eDialogFixed:
    break;
  }

  // Create the dialog
  WindowStyle style;
  style.hide();
  style.control_parent();

  theForm = new WndForm(Parent, sTmp, X, Y, Width, Height, style);

  // Sets Fonts
  if (Font != -1)
    theForm->SetTitleFont(*FontMap[Font]);

  if (Font != -1)
    theForm->SetFont(*FontMap[Font]);

  // Set fore- and background colors
  LoadColors(*theForm, xNode);

  // Load the children controls
  LoadChildrenFromXML(*theForm, theForm, LookUpTable, &xNode,
                      Font, eDialogStyle);

  // If XML error occurred -> Error messagebox + cancel
  if (XMLNode::GlobalError) {
    MessageBoxX(gettext(_T("Error in loading XML dialog")),
                gettext(_T("Dialog error")), MB_OK | MB_ICONEXCLAMATION);

    delete theForm;
    return NULL;
  }

  // Return the created form
  return theForm;
}

static DataField *
LoadDataField(XMLNode node, CallBackTableEntry_t *LookUpTable, 
              const DialogStyle_t eDialogStyle)
{
  TCHAR DataType[32];
  TCHAR DisplayFmt[32];
  TCHAR EditFormat[32];
  double Min, Max, Step;
  int Fine;

  _tcscpy(DataType,
          StringToStringDflt(node.getAttribute(_T("DataType")),
                             _T("")));
  _tcscpy(DisplayFmt,
          StringToStringDflt(node. getAttribute(_T("DisplayFormat")),
                             _T("")));
  _tcscpy(EditFormat,
          StringToStringDflt(node.getAttribute(_T("EditFormat")),
                             _T("")));

  Min = StringToIntDflt(node.getAttribute(_T("Min")), INT_MIN);
  Max = StringToIntDflt(node.getAttribute(_T("Max")), INT_MAX);
  Step = StringToFloatDflt(node.getAttribute(_T("Step")), 1);
  Fine = StringToIntDflt(node.getAttribute(_T("Fine")), 0);

  DataField::DataAccessCallback_t callback = (DataField::DataAccessCallback_t)
    CallBackLookup(LookUpTable,
                   StringToStringDflt(node.getAttribute(_T("OnDataAccess")),
                                      NULL));

  if (_tcsicmp(DataType, _T("enum")) == 0)
    return new DataFieldEnum(EditFormat, DisplayFmt, false, callback);

  if (_tcsicmp(DataType, _T("filereader")) == 0)
    return new DataFieldFileReader(EditFormat, DisplayFmt, callback);

  if (_tcsicmp(DataType, _T("boolean")) == 0)
    return new DataFieldBoolean(EditFormat, DisplayFmt, false,
                                _T("ON"), _T("OFF"), callback);

  if (_tcsicmp(DataType, _T("double")) == 0)
    return new DataFieldFloat(EditFormat, DisplayFmt, Min, Max, 0, Step, Fine,
                              callback);

  if (_tcsicmp(DataType, _T("integer")) == 0)
    return new DataFieldInteger(EditFormat, DisplayFmt, (int)Min, (int)Max,
                                (int)0, (int)Step, callback);

  if (_tcsicmp(DataType, _T("string")) == 0)
    return new DataFieldString(EditFormat, DisplayFmt, _T(""), callback);

  return NULL;
}

/**
 * Creates a control from the given XMLNode as a child of the given parent
 * ContainerControl.
 *
 * @param form the WndForm object
 * @param Parent The parent ContainerControl
 * @param LookUpTable The parent CallBackTable
 * @param node The XMLNode that represents the control
 * @param ParentFont The parent's font array index
 * @param eDialogStyle The parent's dialog style
 */
static Window *
LoadChild(WndForm &form, ContainerControl *Parent,
          CallBackTableEntry_t *LookUpTable,
          XMLNode node, int ParentFont, const DialogStyle_t eDialogStyle)
{
  int X, Y, Width, Height, Font;
  TCHAR Caption[128];
  TCHAR Name[64];

  Window *window = NULL;
  WindowControl *WC = NULL;

  // Determine name, coordinates, width, height,
  // font and caption of the control
  GetDefaultWindowControlProps(&node, Name, &X, &Y, &Width, &Height,
                               &Font, Caption, eDialogStyle);

  // Determine the control's font (default = parent's font)
  Font = StringToIntDflt(node.getAttribute(_T("Font")), ParentFont);

  WindowStyle style;

  if (!StringToIntDflt(node.getAttribute(_T("Visible")), 1))
    style.hide();

  if (StringToIntDflt(node.getAttribute(_T("Border")), 0))
    style.border();

  bool advanced = _tcschr(Caption, _T('*')) != NULL;

  // PropertyControl (WndProperty)
  if (_tcscmp(node.getName(), _T("WndProperty")) == 0) {
    WndProperty *W;
    int CaptionWidth;
    int ReadOnly;
    int MultiLine;

    // Determine the width of the caption field
    CaptionWidth = 
      Scale_Dlg_Width(StringToIntDflt(node.getAttribute(_T("CaptionWidth")), 0),
                      eDialogStyle);

    // Determine whether the control is multiline or readonly
    MultiLine = StringToIntDflt(node.getAttribute(_T("MultiLine")), 0);
    ReadOnly = StringToIntDflt(node.getAttribute(_T("ReadOnly")), 0);

    // Load the event callback properties
    WndProperty::DataChangeCallback_t DataNotifyCallback =
      (WndProperty::DataChangeCallback_t)
      CallBackLookup(LookUpTable,
                     StringToStringDflt(node.getAttribute(_T("OnDataNotify")),
                                        NULL));

    WindowControl::OnHelpCallback_t OnHelpCallback =
      (WindowControl::OnHelpCallback_t)
      CallBackLookup(LookUpTable,
                     StringToStringDflt(node.getAttribute(_T("OnHelp")),
                                        NULL));

    // TODO code: Temporary double handling to fix "const unsigned
    // short *" to "unsigned short *" problem

    // Create the Property Control

    style.control_parent();

    EditWindowStyle edit_style;
    if (ReadOnly)
      edit_style.read_only();
    else
      edit_style.tab_stop();

    if (is_embedded())
      /* sunken edge doesn't fit well on the tiny screen of an
         embedded device */
      edit_style.border();
    else
      edit_style.sunken_edge();

    if (MultiLine) {
      edit_style.multiline();
      edit_style.vscroll();
    }

    WC = W = new WndProperty(Parent, Caption, X, Y, Width, Height,
                             CaptionWidth,
                             style, edit_style,
                             DataNotifyCallback);

    // Set the help function event callback
    W->SetOnHelpCallback(OnHelpCallback);

    // Load the help text
    W->SetHelpText(StringToStringDflt(node.getAttribute(_T("Help")), _T("")));

    Caption[0] = '\0';

    // If the control has (at least) one DataField child control
    if (node.nChildNode(_T("DataField")) > 0){
      // -> Load the first DataField control
      DataField *data_field =
        LoadDataField(node.getChildNode(_T("DataField"), 0),
                      LookUpTable, eDialogStyle);

      if (data_field != NULL)
        // Tell the Property control about the DataField control
        W->SetDataField(data_field);
    }

  // ButtonControl (WndButton)
  } else if (_tcscmp(node.getName(), _T("WndButton")) == 0) {
    // Determine ClickCallback function
    WndButton::ClickNotifyCallback_t ClickCallback =
      (WndButton::ClickNotifyCallback_t)
      CallBackLookup(LookUpTable,
                     StringToStringDflt(node.getAttribute(_T("OnClick")),
                                        NULL));

    // Create the ButtonControl

    style.tab_stop();

    WC = new WndButton(Parent, Caption, X, Y, Width, Height,
                       style, ClickCallback);

    Caption[0] = '\0';

  // SymbolButtonControl (WndSymbolButton) not used yet
  } else if (_tcscmp(node.getName(), _T("WndSymbolButton")) == 0) {
    // Determine ClickCallback function
    WndButton::ClickNotifyCallback_t ClickCallback =
      (WndButton::ClickNotifyCallback_t)
      CallBackLookup(LookUpTable,
                     StringToStringDflt(node.getAttribute(_T("OnClick")),
                                        NULL));

    // Create the SymbolButtonControl

    style.tab_stop();

    WC = new WndSymbolButton(Parent, Caption, X, Y, Width, Height,
                             style, ClickCallback);

    Caption[0] = '\0';

#ifndef ALTAIRSYNC
  // EventButtonControl (WndEventButton) not used yet
  } else if (_tcscmp(node.getName(), _T("WndEventButton")) == 0) {
    TCHAR iename[100];
    TCHAR ieparameters[100];
    _tcscpy(iename,
            StringToStringDflt(node.getAttribute(_T("InputEvent")), _T("")));
    _tcscpy(ieparameters,
            StringToStringDflt(node.getAttribute(_T("Parameters")), _T("")));

    // Create the EventButtonControl

    style.tab_stop();

    WC = new WndEventButton(Parent, Caption, X, Y, Width, Height,
                            style,
                            iename, ieparameters);

    Caption[0] = '\0';
#endif

  // PanelControl (WndPanel)
  } else if (_tcscmp(node.getName(), _T("Panel")) == 0) {
    // Create the PanelControl

    style.control_parent();

    PanelControl *frame = new PanelControl(Parent,
                                           X, Y, Width, Height, style);
    WC = frame;

    // Load children controls from the XMLNode
    LoadChildrenFromXML(form, frame, LookUpTable, &node,
                        ParentFont, eDialogStyle);

  // KeyboardControl
  } else if (_tcscmp(node.getName(), _T("Keyboard")) == 0) {
    style.control_parent();

    // Create the KeyboardControl
    KeyboardControl *kb = new KeyboardControl(form, Parent, X, Y, Width, Height, style);
    WC = kb;
    LoadChildrenFromXML(form, kb, LookUpTable, &node,
                        ParentFont, eDialogStyle);

  // DrawControl (WndOwnerDrawFrame)
  } else if (_tcscmp(node.getName(), _T("WndOwnerDrawFrame")) == 0) {
    // Determine DrawCallback function
    WndOwnerDrawFrame::OnPaintCallback_t PaintCallback =
      (WndOwnerDrawFrame::OnPaintCallback_t)
      CallBackLookup(LookUpTable,
                     StringToStringDflt(node.getAttribute(_T("OnPaint")),
                                        NULL));

    // Create the DrawControl
    WC = new WndOwnerDrawFrame(Parent, X, Y, Width, Height,
                               WindowStyle(), PaintCallback);

  // FrameControl (WndFrame)
  } else if (_tcscmp(node.getName(), _T("WndFrame")) == 0){
    // Create the FrameControl
    WC = new WndFrame(Parent, X, Y, Width, Height,
                      WindowStyle());

  // ListBoxControl (WndListFrame)
  } else if (_tcscmp(node.getName(), _T("WndListFrame")) == 0){
    // Determine ItemHeight of the list items
    unsigned item_height =
      Layout::Scale(StringToIntDflt(node.getAttribute(_T("ItemHeight")), 18));

    // Create the ListBoxControl

    style.tab_stop();

    WC = new WndListFrame(Parent, X, Y, Width, Height,
                          style,
                          item_height);

  // TabControl (Tabbed)
  } else if (_tcscmp(node.getName(), _T("Tabbed")) == 0) {
    // Create the TabControl

    style.control_parent();

    TabbedControl *tabbed = new TabbedControl(Parent,
                                              X, Y, Width, Height, style);
    WC = tabbed;

    const unsigned n = node.nChildNode();
    for (unsigned i = 0; i < n; ++i) {
      // Load each child control from the child nodes
      Window *window = LoadChild(form, tabbed, LookUpTable,
                                 node.getChildNode(i), ParentFont,
                                 eDialogStyle);
      if (window != NULL)
        tabbed->AddClient(window);
        continue;
    }
  } else if (_tcscmp(node.getName(), _T("Custom")) == 0) {
    // Create a custom Window object with a callback
    CreateWindowCallback_t create = (CreateWindowCallback_t)
      CallBackLookup(LookUpTable,
                     StringToStringDflt(node.getAttribute(_T("OnCreate")),
                                        _T("")));
    if (create == NULL)
      return NULL;

    window = create(Parent->GetClientAreaWindow(),
                    X, Y, Width, Height, style);
  }

  // If WindowControl has been created
  if (WC != NULL) {
    // Set the font style
    if (Font != -1)
      WC->SetFont(FontMap[Font]);

    // Set the fore- and background color
    LoadColors(*WC, node);

    // If caption hasn't been set -> set it
    if (!string_is_empty(Caption))
      WC->SetCaption(Caption);

    window = WC;
  }

  if (window != NULL) {
    if (!string_is_empty(Name))
      form.AddNamed(Name, window);

    if (advanced)
      form.AddAdvanced(window);

    form.AddDestruct(WC);
  }

  return window;
}

/**
 * Loads the Parent's children Controls from the given XMLNode
 *
 * @param form the WndForm object
 * @param Parent The parent control
 * @param LookUpTable The parents CallBackTable
 * @param Node The XMLNode that represents the parent control
 * @param ParentFont The parent's font array index
 * @param eDialogStyle The parent's dialog style
 */
static void
LoadChildrenFromXML(WndForm &form, ContainerControl *Parent,
                    CallBackTableEntry_t *LookUpTable,
                    XMLNode *Node, int ParentFont, const DialogStyle_t eDialogStyle)
{
  // Get the number of childnodes
  int Count = Node->nChildNode();

  unsigned bottom_most = 0;

  // Iterate through the childnodes
  for (int i = 0; i < Count; i++) {
    // Load each child control from the child nodes
    Window *window = LoadChild(form, Parent, LookUpTable,
                               Node->getChildNode(i), ParentFont,
                               eDialogStyle);
    if (window == NULL)
      continue;

    // If the client doesn't know where to go
    // -> move it below the previous one
    if (window->get_position().top == -1)
      window->move(window->get_position().left, bottom_most);

    bottom_most = window->get_position().bottom;
  }
}
