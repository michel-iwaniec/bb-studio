import l10n from "shared/lib/lang/l10n";
import React, { FC, useCallback, useEffect, useMemo, useState } from "react";
import { MusicTempoSetting } from "store/features/settings/settingsState";
import {
  Option,
  OptionLabelWithInfo,
  Select,
  SelectCommonProps,
} from "ui/form/Select";

interface MusicTempoSelectProps extends SelectCommonProps {
  name: string;
  value?: MusicTempoSetting;
  onChange?: (newId: MusicTempoSetting) => void;
}

export interface MusicTempoOption {
  value: MusicTempoSetting;
  label: string;
}

const musicTempoOptions: MusicTempoOption[] = [
  {
    label: "FT_TEMPO",
    value: "famitracker_tempo",
  },
  {
    label: "FS_TEMPO",
    value: "famistudio_tempo",
  },
];

export const MusicTempoSelect: FC<MusicTempoSelectProps> = ({
  value,
  onChange,
}) => {
  const [currentValue, setCurrentValue] = useState<Option>();
  const musicTempoOptionsInfo: { [key: string]: string } = useMemo(
    () => ({
      famitracker_tempo: l10n("FIELD_FAMITRACKER_TEMPO_INFO"),
      famistudio_tempo: l10n("FIELD_FAMISTUDIO_TEMPO_INFO"),
    }),
    []
  );

  useEffect(() => {
    const currentMusicTempo = musicTempoOptions.find(
      (e) => e.value === value
    );
    if (currentMusicTempo) {
      setCurrentValue(currentMusicTempo);
    }
  }, [value]);

  const onSelectChange = useCallback(
    (newValue: MusicTempoOption) => {
      onChange?.(newValue.value);
    },
    [onChange]
  );

  return (
    <Select
      value={currentValue}
      options={musicTempoOptions}
      onChange={onSelectChange}
      formatOptionLabel={(
        option: Option,
        { context }: { context: "menu" | "value" }
      ) => {
        return (
          <OptionLabelWithInfo
            info={
              context === "menu" ? musicTempoOptionsInfo[option.value] : ""
            }
          >
            {option.label}
            {context === "value"
              ? ` (${musicTempoOptionsInfo[option.value]})`
              : ""}
          </OptionLabelWithInfo>
        );
      }}
    />
  );
};
