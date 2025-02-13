/**********************************************************************

  Audacity: A Digital Audio Editor

  NumericConverter.cpp

  Dominic Mazzoni

  Paul Licameli split from NumericTextCtrl.cpp


********************************************************************//**

\class NumericConverter
\brief NumericConverter has all the time conversion and snapping
functionality that used to live in NumericTextCtrl.  The idea is to have
a GUI-less class which can do the conversions, so that we can use it
in sanpping without having a window created each time.

*//****************************************************************//**

\class BuiltinFormatString
\brief BuiltinFormatString is a structure used in the NumericTextCtrl
and holds both a descriptive name for the string format and a
wxPrintf inspired style format string, optimised for displaying time in
different formats.

**********************************************************************/
#include "NumericConverter.h"
#include "NumericConverterFormats.h"
#include "NumericConverterRegistry.h"

#include "Project.h"

#include "formatters/ParsedNumericConverterFormatter.h"

#include <cmath>
#include <wx/setup.h> // for wxUSE_* macros
#include <wx/wx.h>

//
// ----------------------------------------------------------------------------
// NumericConverter Class
// ----------------------------------------------------------------------------
//

NumericConverter::NumericConverter(const FormatterContext& context, NumericConverterType type,
                                   const NumericFormatSymbol & formatName,
                                   double value)
    : mContext { context }
    , mType { std::move(type) }
{
   ResetMinValue();
   ResetMaxValue();

   SetFormatName(formatName);
   SetValue(value);
}

bool NumericConverter::ParseFormatString(
   const TranslatableString& untranslatedFormat)
{
   mFormatter = CreateParsedNumericConverterFormatter(
      mContext, mType, untranslatedFormat);

   return mFormatter != nullptr;
}

NumericConverter::~NumericConverter()
{
}

void NumericConverter::ValueToControls()
{
   ValueToControls(mValue);
}

void NumericConverter::ValueToControls(double rawValue, bool nearest /* = true */)
{
   if (!mFormatter)
      return;

   auto result = mFormatter->ValueToString(rawValue, nearest);

   mValueString = std::move(result.valueString);
   mFieldValueStrings = std::move(result.fieldValueStrings);
}

void NumericConverter::ControlsToValue()
{
   if (!mFormatter)
   {
      mValue = mInvalidValue;
      return;
   }

   auto result = mFormatter->StringToValue(mValueString);

   mValue = result.has_value() ?
               std::clamp(*result, mMinValue, mMaxValue) :
               mInvalidValue;
}

bool NumericConverter::SetFormatName(const NumericFormatSymbol& formatName)
{
   if (mFormatSymbol == formatName && !formatName.empty())
      return false;

   const auto newFormat = NumericConverterFormats::Lookup(mContext, mType, formatName);

   if (mFormatSymbol == newFormat)
      return false;

   mFormatSymbol = newFormat;
   mCustomFormat = {};

   UpdateFormatter();

   return true;
}

NumericFormatSymbol NumericConverter::GetFormatName() const
{
   return mFormatSymbol;
}

bool NumericConverter::SetCustomFormat(const TranslatableString& customFormat)
{
   if (mCustomFormat == customFormat)
      return false;

   if (!ParseFormatString(customFormat))
      return false;

   mFormatSymbol = {};
   mCustomFormat = customFormat;

   UpdateFormatter();

   return true;
}

void NumericConverter::SetValue(double newValue)
{
   mValue = newValue;
   ValueToControls();
   ControlsToValue();
}

void NumericConverter::SetMinValue(double minValue)
{
   mMinValue = minValue;
   if (mMaxValue < minValue)
      mMaxValue = minValue;
   if (mValue < minValue)
      SetValue(minValue);
}

void NumericConverter::ResetMinValue()
{
   mMinValue = 0.0;
}

void NumericConverter::SetMaxValue(double maxValue)
{
   mMaxValue = maxValue;
   if (mMinValue > maxValue) {
      mMinValue = maxValue;
   }
   if (mValue > maxValue)
      SetValue(maxValue);
}

void NumericConverter::ResetMaxValue()
{
   mMaxValue = std::numeric_limits<double>::max();
}

double NumericConverter::GetValue()
{
   ControlsToValue();
   return mValue;
}

wxString NumericConverter::GetString()
{
   ValueToControls();
   return mValueString;
}

int NumericConverter::GetSafeFocusedDigit(int focusedDigit) const noexcept
{
   if (focusedDigit < 0)
      return int(mFormatter->GetDigitInfos().size() - 1);
   else
      return std::clamp<int>(
         focusedDigit, 0, mFormatter->GetDigitInfos().size() - 1);
}

void NumericConverter::Increment(int focusedDigit)
{
   Adjust(1, 1, focusedDigit);
}

void NumericConverter::Decrement(int focusedDigit)
{
   Adjust(1, -1, focusedDigit);
}

bool NumericConverter::UpdateFormatter()
{
   if (!mFormatSymbol.empty())
   {
      auto formatterItem = NumericConverterRegistry::Find(mContext, mType, mFormatSymbol);

      if (formatterItem == nullptr)
      {
         assert(formatterItem != nullptr);
         return false;
      }

      mFormatter = formatterItem->factory->Create(mContext);
   }
   else if (!mCustomFormat.empty ())
   {
      ParseFormatString(mCustomFormat);
   }

   if (mFormatter)
   {
      mFormatUpdatedSubscription =
         mFormatter->Subscribe([this](auto) { OnFormatUpdated(); });
   }

   OnFormatUpdated();
   return mFormatter != nullptr;
}

void NumericConverter::OnFormatUpdated()
{
   if (!mFormatter)
      return;
   
   ValueToControls();
   ControlsToValue();
}

void NumericConverter::Adjust(int steps, int dir, int focusedDigit)
{
   if (!mFormatter || mFormatter->GetDigitInfos().empty())
      return;
   // It is possible and "valid" for steps to be zero if a
   // high precision device is being used and wxWidgets supports
   // reporting a higher precision...Mac wx3 does.
   if (steps == 0)
      return;

   focusedDigit = GetSafeFocusedDigit(focusedDigit);

   wxASSERT(dir == -1 || dir == 1);
   wxASSERT(steps > 0);
   if (steps < 0)
      steps = -steps;

   while (steps != 0)
   {
      mValue = mFormatter->SingleStep(mValue, focusedDigit, dir > 0);
      steps--;
   }

   mValue = std::clamp(mValue, mMinValue, mMaxValue);

   ValueToControls();
}
