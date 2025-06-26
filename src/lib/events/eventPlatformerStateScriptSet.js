const l10n = require("../helpers/l10n").default;

const id = "EVENT_SET_PLATFORMER_STATE_SCRIPT";
const groups = ["EVENT_GROUP_ENGINE_FIELDS"];
const subGroups = {
  EVENT_GROUP_ENGINE_FIELDS: "GAMETYPE_PLATFORMER",
};

const fields = [
  {
    key: "state",
    label: l10n("FIELD_STATE"),
    type: "select",
    defaultValue: "fallStart",
    options: [
      ["fallStart", l10n("FIELD_FALL_START")],
      ["fallEnd", l10n("FIELD_FALL_END")],
      ["groundStart", l10n("FIELD_GROUND_START")],
      ["groundEnd", l10n("FIELD_GROUND_END")],
      ["jumpStart", l10n("FIELD_JUMP_START")],
      ["jumpEnd", l10n("FIELD_JUMP_END")],
      ["dashStart", l10n("FIELD_DASH_START")],
      ["dashEnd", l10n("FIELD_DASH_END")],
      ["ladderStart", l10n("FIELD_LADDER_START")],
      ["ladderEnd", l10n("FIELD_LADDER_END")],
      ["wallStart", l10n("FIELD_WALL_START")],
      ["wallEnd", l10n("FIELD_WALL_END")],
      ["knockbackStart", l10n("FIELD_KNOCKBACK_START")],
      ["knockbackEnd", l10n("FIELD_KNOCKBACK_END")],
      ["blankStart", l10n("FIELD_BLANK_START")],
      ["blankEnd", l10n("FIELD_BLANK_END")],
      ["dashReady", l10n("FIELD_DASH_READY")],
    ],
  },
  {
    key: "__scriptTabs",
    type: "tabs",
    defaultValue: "scriptinput",
    values: {
      scriptinput: l10n("FIELD_ON_STATE"),
    },
  },
  {
    key: "script",
    label: l10n("FIELD_ON_STATE"),
    description: l10n("FIELD_ON_STATE"),
    type: "events",
    allowedContexts: ["global", "entity"],
    conditions: [
      {
        key: "__scriptTabs",
        in: [undefined, "scriptinput"],
      },
    ],
  },
];

const valuesMap = {
  fallStart: "PLATFORM_FALL_INIT",
  fallEnd: "PLATFORM_FALL_END",
  groundStart: "PLATFORM_GROUND_INIT",
  groundEnd: "PLATFORM_GROUND_END",
  jumpStart: "PLATFORM_JUMP_INIT",
  jumpEnd: "PLATFORM_JUMP_END",
  dashStart: "PLATFORM_DASH_INIT",
  dashEnd: "PLATFORM_DASH_END",
  dashReady: "PLATFORM_DASH_READY",
  ladderStart: "PLATFORM_LADDER_INIT",
  ladderEnd: "PLATFORM_LADDER_END",
  wallStart: "PLATFORM_WALL_INIT",
  wallEnd: "PLATFORM_WALL_END",
  knockbackStart: "PLATFORM_KNOCKBACK_INIT",
  knockbackEnd: "PLATFORM_KNOCKBACK_END",
  blankStart: "PLATFORM_BLANK_INIT",
  blankEnd: "PLATFORM_BLANK_END",
};

const compile = (input, helpers) => {
  const {
    _compileSubScript,
    _addComment,
    _callNative,
    _stackPushConst,
    _stackPop,
  } = helpers;

  const callbackLabel = valuesMap[input.state] ?? valuesMap.fallStart;

  const scriptRef = _compileSubScript(
    "state",
    input.script,
    "plat_callback_" + callbackLabel,
  );

  const bank = `___bank_${scriptRef}`;
  const ptr = `_${scriptRef}`;

  _addComment("Set Platformer Script");
  _stackPushConst(callbackLabel);
  _stackPushConst(bank);
  _stackPushConst(ptr);
  _callNative("plat_callback_attach");
  _stackPop(3);
};

module.exports = {
  id,
  groups,
  subGroups,
  fields,
  compile,
  allowedBeforeInitFade: true,
};
