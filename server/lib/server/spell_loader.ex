defmodule ElixirTcpServer.SpellLoader do
  use GenServer
  require Logger

  @spell_file "C:/EQ/spells_us.txt"
  @dets_table :spells
  @ets_table :spells
defstruct [
  :id,
  :name,
  :player_1,
  :teleport_zone,
  :you_cast,
  :cast_on_you,
  :cast_on_other,
  :spell_fades,
  :range,
  :pushback,
  :pushup,
  :cast_time,
  :recovery_time,
  :recast_time,
  :buffdurationformula,
  :buffduration,
  :ae_duration,
  :mana,
  :effect_base_value1,
  :effect_base_value2,
  :effect_base_value3,
  :effect_base_value4,
  :effect_base_value5,
  :effect_base_value6,
  :effect_base_value7,
  :effect_base_value8,
  :effect_base_value9,
  :effect_base_value10,
  :effect_base_value11,
  :effect_base_value12,
  :effect_limit_value1,
  :effect_limit_value2,
  :effect_limit_value3,
  :effect_limit_value4,
  :effect_limit_value5,
  :effect_limit_value6,
  :effect_limit_value7,
  :effect_limit_value8,
  :effect_limit_value9,
  :effect_limit_value10,
  :effect_limit_value11,
  :effect_limit_value12,
  :max1,
  :max2,
  :max3,
  :max4,
  :max5,
  :max6,
  :max7,
  :max8,
  :max9,
  :max10,
  :max11,
  :max12,
  :icon,
  :components1,
  :components2,
  :components3,
  :components4,
  :component_counts1,
  :component_counts2,
  :component_counts3,
  :component_counts4,
  :noexpend_reagent1,
  :noexpend_reagent2,
  :noexpend_reagent3,
  :noexpend_reagent4,
  :formula1,
  :formula2,
  :formula3,
  :formula4,
  :formula5,
  :formula6,
  :formula7,
  :formula8,
  :formula9,
  :formula10,
  :formula11,
  :formula12,
  :light_type,
  :activated,
  :resist_type,
  :effectid1,
  :effectid2,
  :effectid3,
  :effectid4,
  :effectid5,
  :effectid6,
  :effectid7,
  :effectid8,
  :effectid9,
  :effectid10,
  :effectid11,
  :effectid12,
  :target_type,
  :base_diff,
  :skill,
  :zone_type,
  :environment_type,
  :time_of_day,
  :classes1,
  :classes2,
  :classes3,
  :classes4,
  :classes5,
  :classes6,
  :classes7,
  :classes8,
  :classes9,
  :classes10,
  :classes11,
  :classes12,
  :classes13,
  :classes14,
  :classes15,
  :classes16,
  :target_anim,
  :travel_type,
  :spell_affect_index,
  :disallow_sit,
  :deities1,
  :deities2,
  :deities3,
  :deities4,
  :deities5,
  :deities6,
  :deities7,
  :deities8,
  :deities9,
  :deities10,
  :deities11,
  :deities12,
  :deities13,
  :deities14,
  :deities15,
  :field142,
  :field143,
  :new_icon,
  :spell_anim,
  :uninterruptable,
  :resist_diff,
  :hate_adjustment,
  :deleteable,
  :recourse_link,
  :no_partial_resist,
  :field152,
  :field153,
  :field154,
  :description,
  :type_desc_num,
  :effect_desc_num,
  :effect_desc_num2,
  :npc_no_los,
  :field160,
  :reflectable,
  :bonushate_mod,
  :field163,
  :field164,
  :ldon_trap,
  :raw
]

  # ──────────────────────────────────────────────
  # Public API
  # ──────────────────────────────────────────────

  def start_link(_), do: GenServer.start_link(__MODULE__, nil, name: __MODULE__)

  def get_spell_name(id) do
    case :ets.lookup(@ets_table, id) do
      [{^id, spell}] -> spell.name
      _ -> nil
    end
  end

  def get_spell(filters) when is_list(filters) do
    match_map = Map.new(filters)

    :ets.tab2list(@ets_table)
    |> Enum.map(fn {_id, spell} -> spell end)
    |> Enum.find(fn spell ->
      Enum.all?(match_map, fn {k, v} -> Map.get(spell, k) == v end)
    end)
  end

  # ──────────────────────────────────────────────
  # GenServer Callbacks
  # ──────────────────────────────────────────────

  def init(_) do
    :ets.new(@ets_table, [:named_table, :set, :public, read_concurrency: true])

    case :dets.open_file(@dets_table, [file: ~c"spells.dets"]) do
      {:ok, _} ->
        ensure_spells_loaded()
        load_to_ets()
        :dets.close(@dets_table)

      {:error, reason} ->
        Logger.error("❌ Failed to open DETS: #{inspect(reason)}")
    end

    Logger.info("📚 SpellLoader initialized")
    {:ok, nil}
  end

  # ──────────────────────────────────────────────
  # Internal Helpers
  # ──────────────────────────────────────────────

  defp ensure_spells_loaded do
    if :dets.info(@dets_table, :size) == 0 do
      Logger.info("📂 spells.dets empty — importing from file")
      load_spells_from_file()
    else
      Logger.debug("📦 spells.dets already populated")
    end
  end

  defp load_spells_from_file do
    case File.read(@spell_file) do
      {:ok, content} ->
        parse_spell_lines(content)
        |> Enum.each(fn %{id: id} = spell ->
          :dets.insert_new(@dets_table, {id, spell})
        end)

        Logger.info("✅ Spells loaded into DETS")

      {:error, reason} ->
        Logger.error("❌ Failed to read spell file: #{inspect(reason)}")
    end
  end

  def load_to_ets do
    :dets.foldl(fn {id, spell}, acc ->
      :ets.insert(@ets_table, {id, spell})
      acc
    end, :ok, @dets_table)

    Logger.info("⚡ Loaded spells from DETS into ETS")
  end

  # ──────────────────────────────────────────────
  # Spell Parsing
  # ──────────────────────────────────────────────

  def parse_spell_lines(content) when is_binary(content) do
    content
    |> String.split("\r\n", trim: true)
    |> Enum.map(&parse_line/1)
    |> Enum.reject(&is_nil/1)
  end

  defp parse_line(line) do
    case String.split(line, "^") do
      [id_str, name | fields] ->
        with {id, ""} <- Integer.parse(id_str) do
          %{
            id: id,
            name: name,
            type: Enum.at(fields, 27),
            target: Enum.at(fields, 33),
            mana: Enum.at(fields, 76),
            raw: line
          }
        else
          _ -> nil
        end

      _ -> nil
    end
  end
defp from_fields(fields, raw) do
  %__MODULE__{
    id: Enum.at(fields, 0) |> parse_int(),
    name: Enum.at(fields, 1),
    player_1: Enum.at(fields, 2),
    teleport_zone: Enum.at(fields, 3),
    you_cast: Enum.at(fields, 4),
    cast_on_you: Enum.at(fields, 6),
    cast_on_other: Enum.at(fields, 7),
    spell_fades: Enum.at(fields, 8),
    range: Enum.at(fields, 9),
    pushback: Enum.at(fields, 11),
    pushup: Enum.at(fields, 12),
    cast_time: Enum.at(fields, 13),
    recovery_time: Enum.at(fields, 14),
    recast_time: Enum.at(fields, 15),
    buffdurationformula: Enum.at(fields, 16),
    buffduration: Enum.at(fields, 17),
    ae_duration: Enum.at(fields, 18),
    mana: Enum.at(fields, 19),
    effect_base_value1: Enum.at(fields, 20),
    effect_base_value2: Enum.at(fields, 21),
    effect_base_value3: Enum.at(fields, 22),
    effect_base_value4: Enum.at(fields, 23),
    effect_base_value5: Enum.at(fields, 24),
    effect_base_value6: Enum.at(fields, 25),
    effect_base_value7: Enum.at(fields, 26),
    effect_base_value8: Enum.at(fields, 27),
    effect_base_value9: Enum.at(fields, 28),
    effect_base_value10: Enum.at(fields, 29),
    effect_base_value11: Enum.at(fields, 30),
    effect_base_value12: Enum.at(fields, 31),
    effect_limit_value1: Enum.at(fields, 32),
    effect_limit_value2: Enum.at(fields, 33),
    effect_limit_value3: Enum.at(fields, 34),
    effect_limit_value4: Enum.at(fields, 35),
    effect_limit_value5: Enum.at(fields, 36),
    effect_limit_value6: Enum.at(fields, 37),
    effect_limit_value7: Enum.at(fields, 38),
    effect_limit_value8: Enum.at(fields, 39),
    effect_limit_value9: Enum.at(fields, 40),
    effect_limit_value10: Enum.at(fields, 41),
    effect_limit_value11: Enum.at(fields, 42),
    effect_limit_value12: Enum.at(fields, 43),
    max1: Enum.at(fields, 44),
    max2: Enum.at(fields, 45),
    max3: Enum.at(fields, 46),
    max4: Enum.at(fields, 47),
    max5: Enum.at(fields, 48),
    max6: Enum.at(fields, 49),
    max7: Enum.at(fields, 50),
    max8: Enum.at(fields, 51),
    max9: Enum.at(fields, 52),
    max10: Enum.at(fields, 53),
    max11: Enum.at(fields, 54),
    max12: Enum.at(fields, 55),
    icon: Enum.at(fields, 56),
    components1: Enum.at(fields, 58),
    components2: Enum.at(fields, 59),
    components3: Enum.at(fields, 60),
    components4: Enum.at(fields, 61),
    component_counts1: Enum.at(fields, 62),
    component_counts2: Enum.at(fields, 63),
    component_counts3: Enum.at(fields, 64),
    component_counts4: Enum.at(fields, 65),
    noexpend_reagent1: Enum.at(fields, 66),
    noexpend_reagent2: Enum.at(fields, 67),
    noexpend_reagent3: Enum.at(fields, 68),
    noexpend_reagent4: Enum.at(fields, 69),
    formula1: Enum.at(fields, 70),
    formula2: Enum.at(fields, 71),
    formula3: Enum.at(fields, 72),
    formula4: Enum.at(fields, 73),
    formula5: Enum.at(fields, 74),
    formula6: Enum.at(fields, 75),
    formula7: Enum.at(fields, 76),
    formula8: Enum.at(fields, 77),
    formula9: Enum.at(fields, 78),
    formula10: Enum.at(fields, 79),
    formula11: Enum.at(fields, 80),
    formula12: Enum.at(fields, 81),
    light_type: Enum.at(fields, 82),
    activated: Enum.at(fields, 83),
    resist_type: Enum.at(fields, 84),
    effectid1: Enum.at(fields, 85),
    effectid2: Enum.at(fields, 86),
    effectid3: Enum.at(fields, 87),
    effectid4: Enum.at(fields, 88),
    effectid5: Enum.at(fields, 89),
    effectid6: Enum.at(fields, 90),
    effectid7: Enum.at(fields, 91),
    effectid8: Enum.at(fields, 92),
    effectid9: Enum.at(fields, 93),
    effectid10: Enum.at(fields, 94),
    effectid11: Enum.at(fields, 95),
    effectid12: Enum.at(fields, 96),
    target_type: Enum.at(fields, 97),
    base_diff: Enum.at(fields, 98),
    skill: Enum.at(fields, 99),
    zone_type: Enum.at(fields, 100),
    environment_type: Enum.at(fields, 101),
    time_of_day: Enum.at(fields, 102),
    classes1: Enum.at(fields, 103),
    classes2: Enum.at(fields, 104),
    classes3: Enum.at(fields, 105),
    classes4: Enum.at(fields, 106),
    classes5: Enum.at(fields, 107),
    classes6: Enum.at(fields, 108),
    classes7: Enum.at(fields, 109),
    classes8: Enum.at(fields, 110),
    classes9: Enum.at(fields, 111),
    classes10: Enum.at(fields, 112),
    classes11: Enum.at(fields, 113),
    classes12: Enum.at(fields, 114),
    classes13: Enum.at(fields, 115),
    classes14: Enum.at(fields, 116),
    classes15: Enum.at(fields, 117),
    classes16: Enum.at(fields, 118),
    target_anim: Enum.at(fields, 119),
    travel_type: Enum.at(fields, 120),
    spell_affect_index: Enum.at(fields, 121),
    disallow_sit: Enum.at(fields, 122),
    deities1: Enum.at(fields, 123),
    deities2: Enum.at(fields, 124),
    deities3: Enum.at(fields, 125),
    deities4: Enum.at(fields, 126),
    deities5: Enum.at(fields, 127),
    deities6: Enum.at(fields, 128),
    deities7: Enum.at(fields, 129),
    deities8: Enum.at(fields, 130),
    deities9: Enum.at(fields, 131),
    deities10: Enum.at(fields, 132),
    deities11: Enum.at(fields, 133),
    deities12: Enum.at(fields, 134),
    deities13: Enum.at(fields, 135),
    deities14: Enum.at(fields, 136),
    deities15: Enum.at(fields, 137),
    field142: Enum.at(fields, 142),
    field143: Enum.at(fields, 143),
    new_icon: Enum.at(fields, 144),
    spell_anim: Enum.at(fields, 145),
    uninterruptable: Enum.at(fields, 146),
    resist_diff: Enum.at(fields, 147),
    hate_adjustment: Enum.at(fields, 148),
    deleteable: Enum.at(fields, 149),
    recourse_link: Enum.at(fields, 150),
    no_partial_resist: Enum.at(fields, 151),
    field152: Enum.at(fields, 152),
    field153: Enum.at(fields, 153),
    field154: Enum.at(fields, 154),
    description: Enum.at(fields, 155),
    type_desc_num: Enum.at(fields, 156),
    effect_desc_num: Enum.at(fields, 157),
    effect_desc_num2: Enum.at(fields, 158),
    npc_no_los: Enum.at(fields, 159),
    field160: Enum.at(fields, 160),
    reflectable: Enum.at(fields, 161),
    bonushate_mod: Enum.at(fields, 162),
    field163: Enum.at(fields, 163),
    field164: Enum.at(fields, 164),
    ldon_trap: Enum.at(fields, 165),
    raw: raw
  }
end

defp parse_int(nil), do: nil
defp parse_int(""), do: nil
defp parse_int(val) when is_binary(val), do: String.to_integer(val)
defp parse_int(val), do: val


end
