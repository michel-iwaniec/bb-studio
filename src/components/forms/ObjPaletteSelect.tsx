import PaletteBlock from "components/forms/PaletteBlock";
import React, { FC } from "react";
import { ObjPalette } from "shared/lib/entities/entitiesTypes";
import {
  OptionLabelWithPreview,
  Select,
  SingleValueWithPreview,
} from "ui/form/Select";
import l10n from "shared/lib/lang/l10n";
import { defaultColors } from "consts";

interface ObjPaletteSelectProps {
  name: string;
  value?: ObjPalette;
  onChange?: (newValue: ObjPalette) => void;
}

interface ObjPaletteOption {
  value: ObjPalette;
  label: string;
  colors: string[];
  nesColors: string[];
}

const options: ObjPaletteOption[] = [
  {
    value: "OBP0",
    label: "0: OBP0",
    colors: ["E8F8E0", "B0F088", "", "202850"],
    nesColors: defaultColors,
  },
  {
    value: "OBP1",
    label: "1: OBP1",
    colors: ["E8F8E0", "509878", "", "202850"],
    nesColors: defaultColors,
  },
];

export const ObjPaletteSelect: FC<ObjPaletteSelectProps> = ({
  name,
  value,
  onChange,
}) => {
  const currentValue = options.find((o) => o.value === value);
  return (
    <Select
      name={name}
      value={currentValue}
      options={options}
      onChange={(newValue: ObjPaletteOption) => {
        onChange?.(newValue.value);
      }}
      formatOptionLabel={(option: ObjPaletteOption) => {
        return (
          <OptionLabelWithPreview
            preview={
              <PaletteBlock type="sprite" colors={option.nesColors} size={20} />
            }
          >
            {l10n("FIELD_PALETTE")} {option.label}
          </OptionLabelWithPreview>
        );
      }}
      components={{
        SingleValue: () => (
          <SingleValueWithPreview
            preview={
              <PaletteBlock
                type="sprite"
                colors={currentValue?.nesColors || []}
                size={20}
              />
            }
          >
            {l10n("FIELD_PALETTE")} {currentValue?.label}
          </SingleValueWithPreview>
        ),
      }}
    />
  );
};

ObjPaletteSelect.defaultProps = {
  name: undefined,
  value: "OBP0",
};
