/**********************************************************************

  Audacity: A Digital Audio Editor

  SelectionBar.h

  Dominic Mazzoni
  Dmitry Vedenko

**********************************************************************/

#ifndef __AUDACITY_SELECTION_BAR__
#define __AUDACITY_SELECTION_BAR__

#include <wx/defs.h>

#include <array>

#include "ToolBar.h"
#include "widgets/auStaticText.h"

#include "Observer.h"


class wxChoice;
class wxComboBox;
class wxCommandEvent;
class wxDC;
class wxSizeEvent;
class wxStaticText;

class AudacityProject;
class SelectionBarListener;
class NumericTextCtrl;

extern IntSetting SelectionToolbarMode;

class AUDACITY_DLL_API SelectionBar final : public ToolBar {

 public:
   enum class SelectionMode
   {
      StartEnd,
      StartLength,
      LengthEnd,
      LengthCenter 
   };
   
   static Identifier ID();

   SelectionBar( AudacityProject &project );
   virtual ~SelectionBar();

   bool ShownByDefault() const override;
   DockID DefaultDockID() const override;

   static SelectionBar &Get( AudacityProject &project );
   static const SelectionBar &Get( const AudacityProject &project );

   void Create(wxWindow *parent) override;

   void Populate() override;
   void Repaint(wxDC * WXUNUSED(dc)) override {};
   void EnableDisableButtons() override {};
   void UpdatePrefs() override;

   void SetTimes(double start, double end);

   void SetSelectionFormat(const NumericFormatSymbol & format);
   void SetListener(SelectionBarListener *l);
   void RegenerateTooltips() override;

 private:
   AButton* MakeSetupButton();
   
   void AddTitle( const TranslatableString & Title,
      wxSizer * pSizer );
   void AddTime( int id, wxSizer * pSizer );
   void AddSelectionSetupButton(wxSizer* pSizer);

   void SetSelectionMode(SelectionMode mode);
   void ValuesToControls();
   void OnUpdate(wxCommandEvent &evt);

   void OnFocus(wxFocusEvent &event);
   void OnCaptureKey(wxCommandEvent &event);
   void OnSize(wxSizeEvent &evt);
   void OnIdle( wxIdleEvent &evt );

   void ModifySelection(int driver, bool done = false);
   void SelectionModeUpdated();

   SelectionBarListener * mListener;
   double mRate;
   double mStart, mEnd, mLength, mCenter;

   SelectionMode mSelectionMode {};
   SelectionMode mLastSelectionMode {};

   std::array<NumericTextCtrl*, 2> mTimeControls {};

   wxString mLastValidText;
   
 public:

   DECLARE_CLASS(SelectionBar)
   DECLARE_EVENT_TABLE()
};

#endif

