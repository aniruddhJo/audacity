/**********************************************************************

  Audacity: A Digital Audio Editor

  NumericConverter.h

  Dominic Mazzoni

  See NumericConverter.cpp for documentation on how to use the
  format string to specify how a NumericTextCtrl's fields are
  laid out.

  Paul Licameli split from NumericTextCtrl.h

**********************************************************************/
#ifndef __AUDACITY_NUMERIC_CONVERTER__
#define __AUDACITY_NUMERIC_CONVERTER__

#include <memory>

#include "NumericConverterType.h"
#include "NumericConverterFormatter.h"
#include "NumericConverterFormatterContext.h"

#include "ComponentInterfaceSymbol.h"
#include "TranslatableString.h"


class NUMERIC_FORMATS_API NumericConverter /* not final */
{
public:
   NumericConverter(const FormatterContext& context, NumericConverterType type,
                    const NumericFormatSymbol & formatName = {},
                    double value = 0.0f);

   virtual ~NumericConverter();  

   // ValueToControls() formats a raw value (either provided as
   // argument, or mValue, depending on the version of the function
   // called). The result is stored to mValueString.
   virtual void ValueToControls();
   virtual void ValueToControls(double rawValue, bool nearest = true);

   // Converts the stored formatted string (mValueString) back to a
   // raw value (mValue).
   virtual void ControlsToValue();

private:
   bool ParseFormatString(const TranslatableString & untranslatedFormat);

public:
   // returns true if the format name really changed:
   bool SetFormatName(const NumericFormatSymbol & formatName);
   // Could be empty if custom format is used
   NumericFormatSymbol GetFormatName() const;

   bool SetCustomFormat(const TranslatableString& customFormat);

   void SetValue(double newValue);
   void SetMinValue(double minValue);
   void ResetMinValue();
   void SetMaxValue(double maxValue);
   void ResetMaxValue();

   double GetValue();
   wxString GetString();

   // Adjust the value by the number "steps" in the active format.
   // Increment if "dir" is 1, decrement if "dir" is -1.
   void Adjust(int steps, int dir, int focusedDigit);

   void Increment(int focusedDigit = -1);
   void Decrement(int focusedDigit = -1);

protected:
   bool UpdateFormatter();
   virtual void OnFormatUpdated();

   FormatterContext mContext;

   const NumericConverterType mType;

   double         mValue;

   double         mMinValue;
   double         mMaxValue;
   double         mInvalidValue { -1 };

   std::unique_ptr<NumericConverterFormatter>
                 mFormatter;
   
   NumericFormatSymbol mFormatSymbol;
   TranslatableString mCustomFormat;

   // Formatted mValue, by ValueToControls().
   wxString       mValueString;
   std::vector<wxString> mFieldValueStrings;

   Observer::Subscription mFormatUpdatedSubscription;

private:
   int GetSafeFocusedDigit(int focusedDigit) const noexcept;
};
#endif // __AUDACITY_NUMERIC_CONVERTER__
