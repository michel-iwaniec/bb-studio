import React, { useCallback, useEffect, useRef, useState } from "react";
import l10n from "shared/lib/lang/l10n";
import ColorSlider from "./ColorSlider";
import { paletteSelectors } from "store/features/entities/entitiesState";
import entitiesActions from "store/features/entities/entitiesActions";
import { Button } from "ui/buttons/Button";
import { hexDec } from "shared/lib/helpers/8bit";
import clamp from "shared/lib/helpers/clamp";
import styled, { css } from "styled-components";
import { TextField } from "ui/form/TextField";
import { NumberField } from "ui/form/NumberField";
import { FixedSpacer } from "ui/spacing/Spacing";
import API from "renderer/lib/api";
import { useAppDispatch, useAppSelector } from "store/hooks";
import { GBCHexToClosestHex } from "shared/lib/color/gbcColors";

import { nesHex_to_rgb888Hex } from "lib/compiler/rgb_to_nes";
import { update } from "lodash";
import { DEFAULT_WHITE, DEFAULT_LIGHT, DEFAULT_DARK, DEFAULT_BLACK, defaultColors } from "consts";

type ColorIndex = 0 | 1 | 2 | 3;

interface CustomPalettePickerProps {
  paletteId: string;
}

const colorIndexes: ColorIndex[] = [0, 1, 2, 3];

type NESColor = number;

const colorSpaceValues: NESColor[] = Array.from(Array(64).keys());

const Wrapper = styled.div`
  display: flex;
  flex-direction: column;
  width: 100%;
  max-width: 1000px;
`;

const ColorsWrapper = styled.div`
  display: grid;
  grid-template-columns: auto auto auto auto;
  gap: 10px;
  margin-bottom: 20px;
`;

const ColorSpaceWrapper = styled.div`
  display: grid;
  grid-template-rows: auto auto auto auto;
  grid-template-columns: auto auto auto auto auto auto auto auto auto auto auto auto auto auto auto auto;
  gap: 4px;
  margin-top: 20px;
  margin-bottom: 20px;
`;

interface ColorButtonProps {
  selected?: boolean;
}

const ColorButton = styled.button<ColorButtonProps>`
  position: relative;
  display: block;
  justify-content: center;
  text-align: center;
  border: 1px solid ${(props) => props.theme.colors.input.border};
  border-radius: ${(props) => props.theme.borderRadius}px;
  max-height: 128px;
  height: 10vh;
  width: 100%;
  font-size: 40px;

  span {
    position: absolute;
    top: -30px;
    white-space: nowrap;
    text-overflow: ellipsis;
    overflow: hidden;
    color: ${(props) => props.theme.colors.text};
    font-size: 16px;
  }

  :hover {
    box-shadow: 0 0 0px 4px ${(props) => props.theme.colors.input.border};
  }

  ${(props) =>
    props.selected
      ? css`
          &,
          :hover {
            box-shadow: 0 0 0px 4px ${(props) => props.theme.colors.highlight};
          }
        `
      : ""}
`;

const CustomPalettePickerNES = ({ paletteId }: CustomPalettePickerProps) => {
  const dispatch = useAppDispatch();

  const palette = useAppSelector((state) =>
    paletteSelectors.selectById(state, paletteId)
  );

  const [selectedColor, setSelectedColor] = useState<ColorIndex>(0);

  const getWhiteHex = useCallback(
    () => (palette?.nesColors || defaultColors)[0],
    [palette?.nesColors]
  );
  const getLightHex = useCallback(
    () => (palette?.nesColors || defaultColors)[1],
    [palette?.nesColors]
  );
  const getDarkHex = useCallback(
    () => (palette?.nesColors || defaultColors)[2],
    [palette?.nesColors]
  );
  const getBlackHex = useCallback(
    () => (palette?.nesColors || defaultColors)[3],
    [palette?.nesColors]
  );

  const updateCurrentColor = useCallback(
    (newHex: string) => {
      if (selectedColor === 0) {
        dispatch(
          entitiesActions.editPalette({
            paletteId,
            changes: {
              nesColors: [newHex, getLightHex(), getDarkHex(), getBlackHex()],
            },
          })
        );
      } else if (selectedColor === 1) {
        dispatch(
          entitiesActions.editPalette({
            paletteId,
            changes: {
              nesColors: [getWhiteHex(), newHex, getDarkHex(), getBlackHex()],
            },
          })
        );
      } else if (selectedColor === 2) {
        dispatch(
          entitiesActions.editPalette({
            paletteId,
            changes: {
              nesColors: [getWhiteHex(), getLightHex(), newHex, getBlackHex()],
            },
          })
        );
      } else if (selectedColor === 3) {
        dispatch(
          entitiesActions.editPalette({
            paletteId,
            changes: {
              nesColors: [getWhiteHex(), getLightHex(), getDarkHex(), newHex],
            },
          })
        );
      }
    },
    [
      dispatch,
      getBlackHex,
      getDarkHex,
      getLightHex,
      getWhiteHex,
      paletteId,
      selectedColor,
    ]
  );

  const onColorSelect = useCallback(
    (colorIndex: ColorIndex) => {
      setSelectedColor(colorIndex);
    },
    []
  );

  const onPickNESColor = useCallback(
    (colorData: NESColor) => {
      const editHex = colorData.toString(16).padStart(2, "0").toUpperCase();
      updateCurrentColor(editHex);
    },
    [updateCurrentColor]
  );

  const luminance = (
    r: number,
    g: number,
    b: number): number => {
    return 0.2126 * r + 0.7152 * g + 0.0722 * b;
  }

  const textColor = (
    bgColor: string
  ): string => {
    const r: number = hexDec(bgColor.substring(0, 2));
    const g: number = hexDec(bgColor.substring(2, 4));
    const b: number = hexDec(bgColor.substring(4, 6));
    if (luminance(r, g, b) < 128) {
      // Pick white foreground color
      return "FFFFFF";
    }
    else {
      // Pick black foreground color
      return "000000";
    }
  }

  if (!palette) {
    return <div />;
  }

  return (
    <Wrapper>
      <ColorsWrapper>
        {colorIndexes.map((index) => (
          <ColorButton
            key={index}
            selected={selectedColor === index}
            className="focus-visible"
            onClick={() => onColorSelect(index)}
            style={{
              background: `#${nesHex_to_rgb888Hex((palette?.nesColors || palette.colors)[index])}`,
              color: `#${textColor(nesHex_to_rgb888Hex((palette?.nesColors || palette.colors)[index]))}`,
              width: "222px",
            }
            }
          >
            {index === 0 && <span>{l10n("FIELD_COLOR_LIGHTEST")}</span>}
            {index === 3 && <span>{l10n("FIELD_COLOR_DARKEST")}</span>}

            {`${(palette?.nesColors || defaultColors)[index]}`}
          </ColorButton>
        ))}
      </ColorsWrapper>
      <ColorSpaceWrapper>
        {colorSpaceValues.map((index) => (
          <ColorButton
            key={index}
            selected={false}
            disabled={index == 0x0D}
            className="focus-visible"
            onClick={() => onPickNESColor(index)}
            style={{
              background: `#${nesHex_to_rgb888Hex(index.toString(16).padStart(2, "0").toUpperCase())}`,
              fontSize: "30px",
              height: "6vh",
              color: (index == 0x0D) ? "#FF0000" : `#${textColor(nesHex_to_rgb888Hex(index.toString(16).padStart(2, "0").toUpperCase()))}`,
            }
            }
          >
            {(index == 0x0D) ? "XX" : `${index.toString(16).padStart(2, "0").toUpperCase() }`}
          </ColorButton>
        ))}
      </ColorSpaceWrapper>
    </Wrapper>
  );
};

export default CustomPalettePickerNES;
