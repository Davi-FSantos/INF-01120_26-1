#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "src/core/Voice.h"
#include "src/parsing/TextParser.h"
#include <vector>
#include <optional>

// Helper function to drain all events from a Voice
static std::vector<MidiEvent> getEvents(Voice& voice) {
    std::vector<MidiEvent> events;
    while (voice.hasEvents()) {
        auto ev = voice.getNextEvent();
        if (ev) {
            events.push_back(*ev);
        }
    }
    return events;
}

TEST_CASE("Voice Fugue Defaults") {
    // V0 soprano
    Voice v0(0);
    CHECK(v0.currentOctave == 6);
    CHECK(v0.currentVolume == 100);
    CHECK(v0.currentInstrument == 0);
    CHECK(v0.channel == 0);

    // V1 alto
    Voice v1(1);
    CHECK(v1.currentOctave == 5);
    CHECK(v1.currentVolume == 80);
    CHECK(v1.currentInstrument == 20);
    CHECK(v1.channel == 1);

    // V2 tenor
    Voice v2(2);
    CHECK(v2.currentOctave == 4);
    CHECK(v2.currentVolume == 60);
    CHECK(v2.currentInstrument == 6);
    CHECK(v2.channel == 2);

    // V3 bass
    Voice v3(3);
    CHECK(v3.currentOctave == 3);
    CHECK(v3.currentVolume == 40);
    CHECK(v3.currentInstrument == 71);
    CHECK(v3.channel == 3);

    // V4 cycles to V0 properties
    Voice v4(4);
    CHECK(v4.currentOctave == 6);
    CHECK(v4.currentVolume == 100);
    CHECK(v4.currentInstrument == 0);
    CHECK(v4.channel == 4);
}

TEST_CASE("Voice noteToMidiPitch") {
    // A4 is (4+1)*12 + 9 = 69
    CHECK(Voice::noteToMidiPitch('A', 4) == 69);
    // B4 is (4+1)*12 + 11 = 71
    CHECK(Voice::noteToMidiPitch('B', 4) == 71);
    // C4 is (4+1)*12 + 0 = 60
    CHECK(Voice::noteToMidiPitch('C', 4) == 60);
    // D4 is (4+1)*12 + 2 = 62
    CHECK(Voice::noteToMidiPitch('D', 4) == 62);
    // E4 is (4+1)*12 + 4 = 64
    CHECK(Voice::noteToMidiPitch('E', 4) == 64);
    // F4 is (4+1)*12 + 5 = 65
    CHECK(Voice::noteToMidiPitch('F', 4) == 65);
    // G4 is (4+1)*12 + 7 = 67
    CHECK(Voice::noteToMidiPitch('G', 4) == 67);
    // H4 (Bb4) is (4+1)*12 + 10 = 70
    CHECK(Voice::noteToMidiPitch('H', 4) == 70);

    // Invalid note returns -1
    CHECK(Voice::noteToMidiPitch('Z', 4) == -1);

    // Clamping to 0-127
    // Minimum pitch clamping: C at octave -1 (or lower)
    CHECK(Voice::noteToMidiPitch('C', -2) == 0); // (-2+1)*12 + 0 = -12 -> clamped to 0
    // Maximum pitch clamping: H at octave 9 is (9+1)*12 + 10 = 130 -> clamped to 127
    CHECK(Voice::noteToMidiPitch('H', 9) == 127);

    // String overload tests
    CHECK(Voice::noteToMidiPitch(std::string("A"), 4) == 69);
    CHECK(Voice::noteToMidiPitch(std::string("Eb"), 4) == 63); // (4+1)*12 + 3 = 63
    CHECK(Voice::noteToMidiPitch(std::string("Mb"), 4) == 63); // (4+1)*12 + 3 = 63
    CHECK(Voice::noteToMidiPitch(std::string("Ab"), 4) == 68); // (4+1)*12 + 8 = 68
}

TEST_CASE("TextParser - Notes and ProgramChange Events") {
    TextParser parser;
    auto voices = parser.parse("C", 120);
    REQUIRE(voices.size() == 1);
    auto events = getEvents(voices[0]);
    
    // Default instrument for V0 is 0.
    // Events: 1. ProgramChange (value 0, timestamp 0), 2. NoteOn (pitch 84, volume 100, timestamp 0), 3. NoteOff (pitch 84, timestamp 1)
    REQUIRE(events.size() == 3);
    CHECK(events[0].type == MidiEventType::ProgramChange);
    CHECK(events[0].value == 0);
    CHECK(events[0].timestamp == 0.0);

    CHECK(events[1].type == MidiEventType::NoteOn);
    CHECK(events[1].pitch == 84); // C6 = (6+1)*12 = 84
    CHECK(events[1].velocity == 100);
    CHECK(events[1].timestamp == 0.0);

    CHECK(events[2].type == MidiEventType::NoteOff);
    CHECK(events[2].pitch == 84);
    CHECK(events[2].timestamp == 1.0);
}

TEST_CASE("TextParser - Lowercase Rests") {
    TextParser parser;
    auto voices = parser.parse("C a D", 120);
    REQUIRE(voices.size() == 1);
    auto events = getEvents(voices[0]);

    // C NoteOn (0.0), C NoteOff (1.0)
    // ' ' is space -> doubles volume (VolumeChange at 1.0)
    // 'a' is rest -> no note events, increments beat to 2.0
    // ' ' is space -> doubles volume (VolumeChange at 2.0)
    // D NoteOn (2.0), D NoteOff (3.0)
    //
    // Events list:
    // 0: ProgramChange
    // 1: NoteOn C at 0.0
    // 2: NoteOff C at 1.0
    // 3: VolumeChange (200 -> capped 127) at 1.0
    // 4: VolumeChange (127 -> capped 127) at 2.0
    // 5: NoteOn D (pitch 86) at 2.0
    // 6: NoteOff D at 3.0
    REQUIRE(events.size() == 7);
    CHECK(events[1].type == MidiEventType::NoteOn);
    CHECK(events[1].timestamp == 0.0);
    CHECK(events[3].type == MidiEventType::VolumeChange);
    CHECK(events[3].timestamp == 1.0);
    CHECK(events[4].type == MidiEventType::VolumeChange);
    CHECK(events[4].timestamp == 2.0);
    CHECK(events[5].type == MidiEventType::NoteOn);
    CHECK(events[5].timestamp == 2.0);
}

TEST_CASE("TextParser - Volume Adjustments") {
    TextParser parser;
    auto voices = parser.parse("C  ", 120); // Volume 100, then two spaces.
    REQUIRE(voices.size() == 1);
    auto events = getEvents(voices[0]);

    // Initial ProgramChange at 0
    // NoteOn C at 0
    // NoteOff C at 1
    // Space 1: VolumeChange(127) at 1.0
    // Space 2: VolumeChange(127) at 1.0
    REQUIRE(events.size() == 5);
    CHECK(events[3].type == MidiEventType::VolumeChange);
    CHECK(events[3].value == 127);
    CHECK(events[3].timestamp == 1.0);
    CHECK(events[4].type == MidiEventType::VolumeChange);
    CHECK(events[4].value == 127);
    CHECK(events[4].timestamp == 1.0);
}

TEST_CASE("TextParser - Instrument Mappings") {
    TextParser parser;

    SUBCASE("Exclamation Mark !") {
        auto voices = parser.parse("!", 120);
        auto events = getEvents(voices[0]);
        // PC(0) at 0, PC(22) at 0
        REQUIRE(events.size() == 2);
        CHECK(events[1].type == MidiEventType::ProgramChange);
        CHECK(events[1].value == 22); // Harmonica
    }

    SUBCASE("Vowels O, o, I, i, U, u") {
        auto voices = parser.parse("OoIiUu", 120);
        auto events = getEvents(voices[0]);
        // PC(0) at 0, and 6 PC(109) events at 0
        REQUIRE(events.size() == 7);
        for (size_t i = 1; i < events.size(); ++i) {
            CHECK(events[i].type == MidiEventType::ProgramChange);
            CHECK(events[i].value == 109); // Bagpipe
        }
    }

    SUBCASE("Digits (Even and Odd)") {
        auto voices = parser.parse("01234", 120);
        auto events = getEvents(voices[0]);
        // PC(0) at 0
        // '0' -> even -> current + 0 = 0 -> PC(0) at 0
        // '1' -> odd -> PC(15) (Tubular Bells) at 0
        // '2' -> even -> current (15) + 2 = 17 -> PC(17) at 0
        // '3' -> odd -> PC(15) at 0
        // '4' -> even -> current (15) + 4 = 19 -> PC(19) at 0
        REQUIRE(events.size() == 6);
        CHECK(events[1].value == 0);
        CHECK(events[2].value == 15);
        CHECK(events[3].value == 17);
        CHECK(events[4].value == 15);
        CHECK(events[5].value == 19);
    }

    SUBCASE("Semicolon ; and Comma ,") {
        auto voices = parser.parse(";,", 120);
        auto events = getEvents(voices[0]);
        // PC(0), PC(15) [;], PC(20) [,]
        REQUIRE(events.size() == 3);
        CHECK(events[1].value == 15); // Tubular bells
        CHECK(events[2].value == 20); // Church organ
    }
}

TEST_CASE("TextParser - Octave Manipulations") {
    TextParser parser;

    SUBCASE("Octave Increments (? and .)") {
        auto voices = parser.parse("C?C.C", 120);
        auto events = getEvents(voices[0]);
        // C (octave 6, pitch 84)
        // ? (octave -> 7)
        // C (octave 7, pitch 96)
        // . (octave -> 8)
        // C (octave 8, pitch 108)
        REQUIRE(events.size() == 7);
        CHECK(events[1].pitch == 84);
        CHECK(events[3].pitch == 96);
        CHECK(events[5].pitch == 108);
    }

    SUBCASE("Octave Decrements (V)") {
        auto voices = parser.parse("CVC", 120);
        auto events = getEvents(voices[0]);
        // C (octave 6, pitch 84)
        // V (octave -> 5)
        // C (octave 5, pitch 72)
        REQUIRE(events.size() == 5);
        CHECK(events[1].pitch == 84);
        CHECK(events[3].pitch == 72);
    }

    SUBCASE("Octave Clamping and Cycling") {
        // Increment past 9: should reset to default 6
        auto voicesInc = parser.parse("C????C", 120); // 6 -> 7 -> 8 -> 9 -> 6
        auto eventsInc = getEvents(voicesInc[0]);
        CHECK(eventsInc[1].pitch == 84);
        CHECK(eventsInc[3].pitch == 84);

        // Decrement below 0: should clamp to 0
        auto voicesDec = parser.parse("CVVVVVVVC", 120); // 6 -> 5 -> 4 -> 3 -> 2 -> 1 -> 0 -> 0 (clamped)
        auto eventsDec = getEvents(voicesDec[0]);
        CHECK(eventsDec[1].pitch == 84);
        CHECK(eventsDec[3].pitch == 12); // Octave 0 note C is 12
    }
}

TEST_CASE("TextParser - Consonants and Repetition") {
    TextParser parser;

    SUBCASE("Consonant after note repeats note") {
        auto voices = parser.parse("CX", 120); // C followed by consonant X. X should repeat C.
        auto events = getEvents(voices[0]);
        REQUIRE(events.size() == 5);
        CHECK(events[1].pitch == 84);
        CHECK(events[1].timestamp == 0.0);
        CHECK(events[3].pitch == 84);
        CHECK(events[3].timestamp == 1.0);
    }

    SUBCASE("Consonant after rest behaves as silence") {
        auto voices = parser.parse("aX", 120); // Rest 'a', then consonant X.
        auto events = getEvents(voices[0]);
        REQUIRE(events.size() == 1); // Only initial ProgramChange
        CHECK(voices[0].currentBeat == 2.0); // Rest is 1.0, X is 1.0.
    }
}

TEST_CASE("TextParser - Polyphony and Delays") {
    TextParser parser;
    std::string testText = "[4] C\n[2] D";
    auto voices = parser.parse(testText, 120);
    REQUIRE(voices.size() == 2);

    // Voice 0 ( soprano index 0, default instrument 0, octave 6 )
    // Entry delay is 4 beats.
    auto events0 = getEvents(voices[0]);
    REQUIRE(events0.size() == 3);
    CHECK(events0[0].type == MidiEventType::ProgramChange);
    CHECK(events0[0].timestamp == 4.0);
    CHECK(events0[1].type == MidiEventType::NoteOn);
    CHECK(events0[1].timestamp == 4.0);
    CHECK(events0[1].pitch == 84);

    // Voice 1 ( alto index 1, default instrument 20, octave 5 )
    // Entry delay is 2 beats.
    auto events1 = getEvents(voices[1]);
    REQUIRE(events1.size() == 3);
    CHECK(events1[0].type == MidiEventType::ProgramChange);
    CHECK(events1[0].timestamp == 2.0);
    CHECK(events1[1].type == MidiEventType::NoteOn);
    CHECK(events1[1].timestamp == 2.0);
    CHECK(events1[1].pitch == 74); // D5 = (5+1)*12 + 2 = 74
}

TEST_CASE("TextParser - Global BPM Control") {
    TextParser parser;
    auto voices = parser.parse(">C<D", 120);
    REQUIRE(voices.size() == 1);
    auto events = getEvents(voices[0]);

    // Initial ProgramChange at 0
    // '>' -> BPM changes to 130. BpmChange event at 0
    // C NoteOn at 0, NoteOff at 1
    // '<' -> BPM changes to 120. BpmChange event at 1
    // D NoteOn at 1, NoteOff at 2
    //
    // Events list:
    // 0: ProgramChange
    // 1: BpmChange (130) at 0.0
    // 2: NoteOn C at 0.0
    // 3: NoteOff C at 1.0
    // 4: BpmChange (120) at 1.0
    // 5: NoteOn D at 1.0
    // 6: NoteOff D at 2.0
    REQUIRE(events.size() == 7);
    CHECK(events[1].type == MidiEventType::BpmChange);
    CHECK(events[1].value == 130);
    CHECK(events[1].timestamp == 0.0);

    CHECK(events[4].type == MidiEventType::BpmChange);
    CHECK(events[4].value == 120);
    CHECK(events[4].timestamp == 1.0);
}

TEST_CASE("TextParser - Flat Notes (Eb, Ab, Mb)") {
    TextParser parser;
    auto voices = parser.parse("Eb Ab Mb", 120);
    REQUIRE(voices.size() == 1);
    auto events = getEvents(voices[0]);

    // Events:
    // 0: ProgramChange
    // 1: NoteOn Eb (pitch 87, volume 100) at 0.0 (V0 octave 6 -> (6+1)*12 + 3 = 87)
    // 2: NoteOff Eb at 1.0
    // 3: VolumeChange (volume doubled by space) at 1.0
    // 4: NoteOn Ab (pitch 92) at 1.0 (V0 octave 6 -> (6+1)*12 + 8 = 92)
    // 5: NoteOff Ab at 2.0
    // 6: VolumeChange (volume doubled by space) at 2.0
    // 7: NoteOn Mb (pitch 87) at 2.0
    // 8: NoteOff Mb at 3.0
    REQUIRE(events.size() == 9);
    
    CHECK(events[1].type == MidiEventType::NoteOn);
    CHECK(events[1].pitch == 87);
    CHECK(events[1].timestamp == 0.0);

    CHECK(events[4].type == MidiEventType::NoteOn);
    CHECK(events[4].pitch == 92);
    CHECK(events[4].timestamp == 1.0);

    CHECK(events[7].type == MidiEventType::NoteOn);
    CHECK(events[7].pitch == 87);
    CHECK(events[7].timestamp == 2.0);
}
