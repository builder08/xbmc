/*
 * RealAudio 2.0 (28.8K)
 * Copyright (c) 2003 the ffmpeg project
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef RA288TABLES_H
#define RA288TABLES_H

static const float amptable[8]={ 0.515625, 0.90234375, 1.57910156, 2.76342773,
                         -0.515625,-0.90234375,-1.57910156,-2.76342773 };

static const float codetable[640]={
         0.326171875,        -1.4404296875,        -0.6123046875,        -0.8740234375,        -1.24658203125,
        -2.45703125,        -2.23486328125,        -0.51025390625,         1.419921875,         1.6201171875,
        -1.37646484375,        -1.30712890625,        -0.462890625,        -1.37939453125,        -2.1728515625,
        -3.26123046875,        -0.166015625,         0.7236328125,        -0.623046875,         0.6162109375,
        -0.2744140625,        -3.29931640625,         0.62548828125,         0.08740234375,        -0.6220703125,
        -1.2265625,        -3.4814453125,        -2.40478515625,         3.37548828125,         1.17724609375,
        -1.2099609375,        -0.076171875,         2.28662109375,        -1.89111328125,         0,
        -4.0078125,         1.044921875,        -0.2333984375,        -1.35986328125,         0.26025390625,
         0.92236328125,         1.34716796875,         0.67431640625,        -3.39599609375,        -2.88720703125,
         2.4814453125,        -1.201171875,        -2.8212890625,         0.87744140625,         0.27734375,
        -1.078125,        -1.61572265625,        -2.20849609375,        -3.044921875,        -3.66455078125,
        -1.32763671875,         2.1279296875,        -1.458984375,        -0.56103515625,         1.30078125,
         0.61474609375,         0.48583984375,         1.32373046875,        -1.203125,        -5.0732421875,
         0.8408203125,        -3.69580078125,        -1.3388671875,         1.06005859375,        -1.13720703125,
         0.50390625,         0.36474609375,        -0.4189453125,        -3.8798828125,        -6.27099609375,
         1.5166015625,         2.37109375,        -2.04736328125,        -1.24072265625,         0.50537109375,
         0.9091796875,        -0.46875,        -3.236328125,         0.2001953125,         2.8720703125,
        -1.21728515625,        -1.283203125,        -1.953125,        -0.029296875,         3.5166015625,
        -1.3046875,         0.7060546875,         0.75,                -1.87060546875,         0.60205078125,
        -2.5888671875,         3.375,                 0.77587890625,        -2.04443359375,         1.78955078125,
        -1.6875,        -3.9892578125,        -3.76416015625,         0.67578125,         2.2939453125,
        -2.29443359375,        -3.03173828125,        -5.45703125,         3.95703125,         8.2177734375,
         0.4541015625,         3.419921875,         0.61962890625,        -4.38330078125,         1.25341796875,
         2.27001953125,         5.763671875,         1.68017578125,        -2.76220703125,         0.58544921875,
         1.2412109375,        -0.08935546875,        -4.32568359375,        -3.89453125,         1.5771484375,
        -1.40234375,        -0.98193359375,        -4.74267578125,        -4.09423828125,         6.33935546875,
         1.5068359375,         1.044921875,        -1.796875,        -4.70849609375,        -1.4140625,
        -3.71533203125,         3.18115234375,        -1.11474609375,        -1.2314453125,         3.091796875,
        -1.62744140625,        -2.744140625,        -4.4580078125,        -5.43505859375,         2.70654296875,
        -0.19873046875,        -3.28173828125,        -8.5283203125,        -1.41064453125,         5.6484375,
         1.802734375,         3.318359375,        -0.1279296875,        -5.2958984375,        -0.90625,
         3.55224609375,         6.544921875,        -1.45947265625,        -5.17333984375,         2.41015625,
         0.119140625,        -1.08349609375,         1.296875,         1.84375,        -2.642578125,
        -1.97412109375,        -2.8974609375,         1.04052734375,         0.42138671875,        -1.3994140625,
        -1.6123046875,         0.85107421875,        -0.9794921875,        -0.0625,        -1.001953125,
        -3.10595703125,         1.6318359375,        -0.77294921875,        -0.01025390625,         0.5576171875,
        -1.87353515625,        -0.89404296875,         3.12353515625,         1.24267578125,        -1.390625,
        -4.556640625,        -3.1875,         2.59228515625,         0.9697265625,        -1.09619140625,
        -2.1923828125,         0.365234375,         0.94482421875,        -1.47802734375,        -0.24072265625,
        -4.51904296875,         2.6201171875,         1.55908203125,        -2.19384765625,         0.87109375,
         2.3359375,        -0.1806640625,         0.9111328125,         0.51611328125,        -0.92236328125,
         3.5849609375,        -1.3134765625,        -1.25830078125,         0.330078125,        -0.29833984375,
        -0.2451171875,         1.09130859375,        -0.9033203125,        -0.86767578125,        -1.00048828125,
         0.49365234375,         1.89453125,        -1.20361328125,         1.07861328125,        -0.07421875,
         1.265625,         1.38134765625,         2.728515625,         1.38623046875,        -3.5673828125,
        -1.48876953125,        -2.4013671875,         2.90771484375,         4.49267578125,        -2.17138671875,
         0.34033203125,         1.908203125,         2.8310546875,        -2.17333984375,        -2.267578125,
        -1.03564453125,         2.658203125,        -1.2548828125,         0.15673828125,        -0.5869140625,
         1.3896484375,        -1.0185546875,         1.724609375,         0.2763671875,        -0.345703125,
        -2.08935546875,         0.4638671875,         2.431640625,         1.83056640625,         0.220703125,
        -1.212890625,         1.7099609375,         0.83935546875,        -0.0830078125,         0.1162109375,
        -1.67724609375,         0.12841796875,         1.0322265625,        -0.97900390625,         1.15283203125,
        -3.5830078125,        -0.58984375,         4.56396484375,        -0.59375,        -1.95947265625,
        -6.5908203125,        -0.21435546875,         3.919921875,        -2.06640625,         0.17626953125,
        -1.82080078125,         2.65283203125,         0.978515625,        -2.30810546875,        -0.61474609375,
        -1.9462890625,         3.78076171875,         4.11572265625,        -1.80224609375,        -0.48193359375,
         2.5380859375,        -0.20654296875,         0.5615234375,        -0.62548828125,         0.3984375,
         3.61767578125,         2.00634765625,        -1.92822265625,         1.3134765625,         0.0146484384313,
         0.6083984375,         1.49169921875,        -0.01708984375,        -0.6689453125,        -0.1201171875,
        -0.72705078125,         2.75146484375,        -0.3310546875,        -1.28271484375,         1.5478515625,
         2.3583984375,        -2.23876953125,         0.98046875,        -0.5185546875,         0.39013671875,
        -0.06298828125,         0.35009765625,         2.2431640625,         7.29345703125,         5.2275390625,
         0.20361328125,         1.34716796875,         0.9033203125,        -2.46923828125,        -0.56298828125,
        -1.89794921875,         3.59423828125,        -2.81640625,         2.09228515625,         0.3251953125,
         0.70458984375,        -0.4580078125,         0.009765625,        -1.03466796875,        -0.82861328125,
        -1.8125,        -1.6611328125,        -1.080078125,         0.0537109375,         1.04296875,
        -1.44140625,         0.005859375,        -0.765625,        -1.708984375,        -0.90576171875,
        -0.64208984375,        -0.84521484375,         0.56640625,        -0.2724609375,         0.83447265625,
         0.04296875,        -2.23095703125,         0.0947265625,        -0.2216796875,        -1.44384765625,
        -1.38623046875,        -0.8134765625,        -0.13330078125,         1.017578125,        -0.07568359375,
        -0.09228515625,        -1.16015625,         0.81201171875,        -0.5078125,        -1.19580078125,
        -1.3876953125,        -0.66845703125,         0.310546875,        -0.12109375,        -1.30712890625,
         0.74072265625,         0.03857421875,        -1.47119140625,        -1.79150390625,        -0.47509765625,
         0.93408203125,        -1.21728515625,        -2.59375,        -0.36572265625,         0.62060546875,
        -1.41748046875,        -1.623046875,        -1.833984375,        -1.8017578125,        -0.89306640625,
        -1.42236328125,        -0.75537109375,        -1.34765625,        -0.6865234375,         0.548828125,
         0.900390625,        -0.8955078125,         0.22265625,         0.3447265625,        -2.0859375,
         0.22802734375,        -2.078125,        -0.93212890625,         0.74267578125,         0.5537109375,
        -0.06201171875,        -0.4853515625,        -0.31103515625,        -0.72802734375,        -3.1708984375,
         0.42626953125,        -0.99853515625,        -1.869140625,        -1.36328125,        -0.2822265625,
         1.12841796875,        -0.88720703125,         1.28515625,        -1.490234375,         0.9609375,
         0.31298828125,         0.5830078125,         0.92431640625,         2.00537109375,         3.0966796875,
        -0.02197265625,         0.5849609375,         1.0546875,        -0.70751953125,         1.07568359375,
        -0.978515625,         0.83642578125,         1.7177734375,         1.294921875,         2.07568359375,
         1.43359375,        -1.9375,         0.625,                 0.06396484375,        -0.720703125,
         1.38037109375,         0.00390625,        -0.94140625,         1.2978515625,         1.71533203125,
         1.56201171875,        -0.3984375,         1.31201171875,        -0.85009765625,        -0.68701171875,
         1.439453125,         1.96728515625,         0.1923828125,        -0.12353515625,         0.6337890625,
         2.0927734375,         0.02490234375,        -2.20068359375,        -0.015625,        -0.32177734375,
         1.90576171875,         2.7568359375,        -2.728515625,        -1.265625,         2.78662109375,
        -0.2958984375,         0.6025390625,        -0.78466796875,        -2.53271484375,         0.32421875,
        -0.25634765625,         1.767578125,        -1.0703125,        -1.23388671875,         0.83349609375,
         2.09814453125,        -1.58740234375,        -1.11474609375,         0.396484375,        -1.10546875,
         2.81494140625,         0.2578125,        -1.60498046875,         0.66015625,         0.81640625,
         1.33544921875,         0.60595703125,        -0.53857421875,        -1.59814453125,        -1.66357421875,
         1.96923828125,         0.8046875,        -1.44775390625,        -0.5732421875,         0.705078125,
         0.0361328125,         0.4482421875,         0.97607421875,         0.44677734375,        -0.5009765625,
        -1.21875,        -0.78369140625,         0.9931640625,         1.4404296875,         0.11181640625,
        -1.05859375,         0.99462890625,         0.00732421921566,-0.6171875,        -0.1015625,
        -1.734375,         0.7470703125,         0.28369140625,         0.72802734375,         0.4697265625,
        -1.27587890625,        -1.1416015625,         1.76806640625,        -0.7265625,        -1.06689453125,
        -0.85302734375,         0.03955078125,         2.7041015625,         0.69921875,        -1.10205078125,
        -0.49755859375,         0.42333984375,         0.1044921875,        -1.115234375,        -0.7373046875,
        -0.822265625,         1.375,                -0.11181640625,         1.24560546875,        -0.67822265625,
         1.32177734375,         0.24609375,         0.23388671875,         1.35888671875,        -0.49267578125,
         1.22900390625,        -0.72607421875,        -0.779296875,         0.30322265625,         0.94189453125,
        -0.072265625,         1.0771484375,        -2.09375,         0.630859375,        -0.68408203125,
        -0.25732421875,         0.60693359375,        -1.33349609375,         0.93212890625,         0.625,
         1.04931640625,        -0.73291015625,         1.80078125,         0.2978515625,        -2.24169921875,
         1.6142578125,        -1.64501953125,         0.91552734375,         1.775390625,        -0.59423828125,
         1.2568359375,         1.22705078125,         0.70751953125,        -1.5009765625,        -2.43115234375,
         0.3974609375,         0.8916015625,        -1.21923828125,         2.0673828125,        -1.99072265625,
         0.8125,        -0.107421875,         1.6689453125,         0.4892578125,         0.54443359375,
         0.38134765625,         0.8095703125,         1.91357421875,         2.9931640625,         1.533203125,
         0.560546875,         1.98486328125,         0.740234375,         0.39794921875,         0.09716796875,
         0.58154296875,         1.21533203125,         1.25048828125,         1.18212890625,         1.19287109375,
         0.3759765625,        -2.88818359375,         2.69287109375,        -0.1796875,        -1.56201171875,
         0.5810546875,         0.51123046875,         1.8271484375,         3.38232421875,        -1.02001953125,
         0.142578125,         1.51318359375,         2.103515625,        -0.3701171875,        -1.19873046875,
         0.25537109375,         1.91455078125,         1.974609375,         0.6767578125,         0.04150390625,
         2.13232421875,         0.4912109375,        -0.611328125,        -0.7158203125,        -0.67529296875,
         1.880859375,         0.77099609375,        -0.03759765625,         1.0078125,         0.423828125,
         2.49462890625,         1.42529296875,        -0.0986328125,         0.17529296875,        -0.24853515625,
         1.7822265625,         1.5654296875,         1.12451171875,         0.82666015625,         0.6328125,
         1.41845703125,        -1.90771484375,         0.11181640625,        -0.583984375,        -1.138671875,
         2.91845703125,        -1.75048828125,         0.39306640625,         1.86767578125,        -1.5322265625,
         1.8291015625,        -0.2958984375,         0.02587890625,        -0.13134765625,        -1.61181640625,
         0.2958984375,         0.9853515625,        -0.642578125,         1.984375,         0.1943359375
};

static const float table1[111]={
        0.576690972,        0.580838025,        0.585013986,        0.589219987,        0.59345597,        0.597723007,
        0.602020264,        0.606384277,        0.610748291,        0.615142822,        0.619598389,        0.624084473,
        0.628570557,        0.633117676,        0.637695313,        0.642272949,        0.646911621,        0.651580811,
        0.656280518,        0.66104126,        0.665802002,        0.670593262,        0.675445557,        0.680328369,
        0.685241699,        0.690185547,        0.695159912,        0.700164795,        0.705230713,        0.710327148,
        0.715454102,        0.720611572,        0.725830078,        0.731048584,        0.736328125,        0.741638184,
        0.747009277,        0.752380371,        0.7578125,        0.763305664,        0.768798828,        0.774353027,
        0.779937744,        0.785583496,        0.791229248,        0.796936035,        0.802703857,        0.808502197,
        0.814331055,        0.820220947,        0.826141357,        0.832092285,        0.838104248,        0.844146729,
        0.850250244,        0.856384277,        0.862548828,        0.868774414,        0.875061035,        0.881378174,
        0.88772583,        0.894134521,        0.900604248,        0.907104492,        0.913635254,        0.920227051,
        0.926879883,        0.933563232,        0.940307617,        0.94708252,        0.953918457,        0.96081543,
        0.96774292,        0.974731445,        0.981781006,        0.988861084,        0.994842529,        0.998565674,
        0.999969482,        0.99911499,        0.996002197,        0.990600586,        0.982910156,        0.973022461,
        0.960876465,        0.946533203,        0.930053711,        0.911437988,        0.89074707,        0.868041992,
        0.843322754,        0.816680908,        0.788208008,        0.757904053,        0.725891113,        0.692199707,
        0.656921387,        0.620178223,        0.582000732,        0.542480469,        0.501739502,        0.459838867,
        0.416900635,        0.373016357,        0.328277588,        0.282775879,        0.236663818,        0.189971924,
        0.142852783,        0.0954284668,        0.0477600098
};

static const float table2[38]={
        0.505699992,        0.524200022,        0.54339999,        0.563300014,        0.583953857,        0.60534668,
        0.627502441,        0.650482178,        0.674316406,        0.699005127,        0.724578857,        0.75112915,
        0.778625488,        0.807128906,        0.836669922,        0.86730957,        0.899078369,        0.932006836,
        0.961486816,        0.982757568,        0.995635986,        1,                0.995819092,        0.983154297,
        0.96206665,        0.932769775,        0.895507813,        0.850585938,        0.798400879,        0.739379883,
        0.674072266,        0.602996826,        0.526763916,        0.446014404,        0.361480713,        0.273834229,
        0.183868408,        0.0923461914
};

static const float table1a[36]={
        0.98828125,        0.976699829,        0.965254128,        0.953942537,        0.942763507,        0.931715488,
        0.920796931,        0.910006344,        0.899342179,        0.888803005,        0.878387332,        0.868093729,
        0.857920766,        0.847867012,        0.837931097,        0.828111589,        0.818407178,        0.808816493,
        0.799338162,        0.789970934,        0.780713439,        0.771564424,        0.762522638,        0.753586829,
        0.744755745,        0.736028135,        0.727402806,        0.718878567,        0.710454226,        0.702128589,
        0.693900526,        0.685768902,        0.677732527,        0.669790328,        0.66194123,        0.654184103
};

static const float table2a[10]={
        0.90625,        0.821289063,        0.74432373,        0.674499512,        0.61126709,
        0.553955078,        0.50201416,        0.454956055,        0.41229248,        0.373657227
};

#endif /* RA288TABLES_H */
