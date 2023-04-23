//Maya ASCII 2024 scene
//Name: planet.ma
//Last modified: Sun, Apr 23, 2023 07:12:13 PM
//Codeset: 1252
requires maya "2024";
requires -nodeType "polyPlatonic" "modelingToolkit" "0.0.0.0";
requires "stereoCamera" "10.0";
requires -nodeType "aiOptions" -nodeType "aiAOVDriver" -nodeType "aiAOVFilter" "mtoa" "5.3.1";
requires "stereoCamera" "10.0";
currentUnit -l centimeter -a degree -t film;
fileInfo "application" "maya";
fileInfo "product" "Maya 2024";
fileInfo "version" "2024";
fileInfo "cutIdentifier" "202303310755-f9d705a5fd";
fileInfo "osv" "Windows 10 Pro v2009 (Build: 19045)";
fileInfo "UUID" "B84FD899-401A-F03E-0CDC-A0B6BC8D37A8";
createNode transform -s -n "persp";
	rename -uid "8E72AC0B-8F43-4BD6-A816-C5A00AFEAF84";
	setAttr ".v" no;
	setAttr ".t" -type "double3" -4.4659721617534327 3.6857264741706355 -1.8732815047811826 ;
	setAttr ".r" -type "double3" -38.738352729560091 -119.00000000000054 0 ;
createNode camera -s -n "perspShape" -p "persp";
	rename -uid "E40E7435-BF4F-46EF-B0F0-3DB70A7EBA05";
	setAttr -k off ".v" no;
	setAttr ".fl" 34.999999999999986;
	setAttr ".coi" 7.6299661127522187;
	setAttr ".imn" -type "string" "persp";
	setAttr ".den" -type "string" "persp_depth";
	setAttr ".man" -type "string" "persp_mask";
	setAttr ".tp" -type "double3" 2.2649765014648438e-06 3.7252902984619141e-09 2.1457672119140625e-06 ;
	setAttr ".hc" -type "string" "viewSet -p %camera";
createNode transform -s -n "top";
	rename -uid "64A31AC6-1241-F1E1-786E-069110812A68";
	setAttr ".v" no;
	setAttr ".t" -type "double3" 0 1000.1 0 ;
	setAttr ".r" -type "double3" -89.999999999999986 0 0 ;
createNode camera -s -n "topShape" -p "top";
	rename -uid "259CEE65-8241-3B59-9148-FBBB4E17A085";
	setAttr -k off ".v" no;
	setAttr ".rnd" no;
	setAttr ".coi" 1000.1;
	setAttr ".ow" 30;
	setAttr ".imn" -type "string" "top";
	setAttr ".den" -type "string" "top_depth";
	setAttr ".man" -type "string" "top_mask";
	setAttr ".hc" -type "string" "viewSet -t %camera";
	setAttr ".o" yes;
createNode transform -s -n "front";
	rename -uid "81E18A01-1E42-3937-082B-1184C674CBD0";
	setAttr ".v" no;
	setAttr ".t" -type "double3" 0 0 1000.1 ;
createNode camera -s -n "frontShape" -p "front";
	rename -uid "66167838-BA48-B878-8B83-B4A501EB3C50";
	setAttr -k off ".v" no;
	setAttr ".rnd" no;
	setAttr ".coi" 1000.1;
	setAttr ".ow" 30;
	setAttr ".imn" -type "string" "front";
	setAttr ".den" -type "string" "front_depth";
	setAttr ".man" -type "string" "front_mask";
	setAttr ".hc" -type "string" "viewSet -f %camera";
	setAttr ".o" yes;
createNode transform -s -n "side";
	rename -uid "C4534AAB-A548-54B1-5033-96A43746960F";
	setAttr ".v" no;
	setAttr ".t" -type "double3" 1000.1 0 0 ;
	setAttr ".r" -type "double3" 0 89.999999999999986 0 ;
createNode camera -s -n "sideShape" -p "side";
	rename -uid "380684EA-7A40-0B23-E4D1-60B7ACED5869";
	setAttr -k off ".v" no;
	setAttr ".rnd" no;
	setAttr ".coi" 1000.1;
	setAttr ".ow" 30;
	setAttr ".imn" -type "string" "side";
	setAttr ".den" -type "string" "side_depth";
	setAttr ".man" -type "string" "side_mask";
	setAttr ".hc" -type "string" "viewSet -s %camera";
	setAttr ".o" yes;
createNode transform -n "planet";
	rename -uid "D2B14B87-E64A-83E5-0DEA-D49F2D2B5928";
createNode mesh -n "planetShape" -p "planet";
	rename -uid "37F1DC71-AD4B-4641-313F-0F96ABBA7D6F";
	setAttr -k off ".v";
	setAttr ".vir" yes;
	setAttr ".vif" yes;
	setAttr ".uvst[0].uvsn" -type "string" "map1";
	setAttr ".cuvs" -type "string" "map1";
	setAttr ".dcc" -type "string" "Ambient+Diffuse";
	setAttr ".ds" no;
	setAttr ".covm[0]"  0 1 1;
	setAttr ".cdvm[0]"  0 1 1;
createNode transform -n "ring";
	rename -uid "5AC32BE9-4331-5F7D-8457-5794F14CFF7F";
createNode mesh -n "ringShape" -p "ring";
	rename -uid "C8003949-4D9E-3E33-E7D8-DFAA73564BE2";
	setAttr -k off ".v";
	setAttr ".vir" yes;
	setAttr ".vif" yes;
	setAttr ".pv" -type "double2" 0.5 1 ;
	setAttr ".uvst[0].uvsn" -type "string" "map1";
	setAttr ".cuvs" -type "string" "map1";
	setAttr ".dcc" -type "string" "Ambient+Diffuse";
	setAttr ".covm[0]"  0 1 1;
	setAttr ".cdvm[0]"  0 1 1;
	setAttr -s 100 ".pt[0:99]" -type "float3"  -0.7720952 0 0.097538128 
		-0.75378227 0 0.19353819 -0.72358167 0 0.286486 -0.68196982 0 0.37491572 -0.62960291 
		0 0.45743296 -0.56730658 0 0.532736 -0.49606377 0 0.59963763 -0.41699758 0 0.65708262 
		-0.33135509 0 0.70416498 -0.24048701 0 0.74014199 -0.14582622 0 0.76444685 -0.048865747 
		0 0.77669579 0.04886537 0 0.77669567 0.14582583 0 0.76444679 0.24048662 0 0.74014181 
		0.33135456 0 0.70416468 0.4169971 0 0.65708226 0.49606326 0 0.59963739 0.5673061 
		0 0.53273582 0.62960207 0 0.45743272 0.68196929 0 0.3749156 0.72358114 0 0.28648576 
		0.75378174 0 0.19353797 0.77209467 0 0.097537987 0.77823114 0 -2.8796438e-07 0.77209461 
		0 -0.097538546 0.75378168 0 -0.19353856 0.72358096 0 -0.28648636 0.68196911 0 -0.37491605 
		0.62960196 0 -0.45743307 0.56730586 0 -0.53273636 0.49606305 0 -0.59963781 0.41699687 
		0 -0.65708268 0.33135444 0 -0.70416498 0.24048632 0 -0.74014199 0.14582564 0 -0.76444685 
		0.048865229 0 -0.77669579 -0.048865855 0 -0.77669573 -0.14582627 0 -0.76444679 -0.24048698 
		0 -0.74014175 -0.33135498 0 -0.70416468 -0.41699722 0 -0.65708238 -0.49606332 0 -0.59963757 
		-0.5673064 0 -0.53273588 -0.62960237 0 -0.45743293 -0.68196929 0 -0.37491572 -0.72358114 
		0 -0.28648615 -0.75378174 0 -0.19353834 -0.77209467 0 -0.097538404 -0.77823114 0 
		-2.1258678e-07 -0.9535321 0 0.12045876 -0.93091571 0 0.2390181 -0.8936184 0 0.35380808 
		-0.84222782 0 0.463018 -0.77755511 0 0.56492603 -0.70061952 0 0.65792489 -0.61263496 
		0 0.74054766 -0.51498914 0 0.81149209 -0.40922108 0 0.86963803 -0.29699963 0 0.91406989 
		-0.18009442 0 0.94408572 -0.060348906 0 0.95921296 0.060348272 0 0.95921296 0.18009369 
		0 0.94408572 0.29699895 0 0.91406977 0.40922031 0 0.86963791 0.51498806 0 0.81149197 
		0.61263412 0 0.74054742 0.70061857 0 0.65792429 0.7775535 0 0.56492573 0.84222651 
		0 0.46301791 0.89361703 0 0.35380751 0.93091452 0 0.23901793 0.95353103 0 0.12045856 
		0.96110964 0 -4.3574209e-07 0.95353097 0 -0.12045943 0.93091452 0 -0.23901871 0.8936168 
		0 -0.35380867 0.84222639 0 -0.46301848 0.77755338 0 -0.56492651 0.70061809 0 -0.65792555 
		0.61263388 0 -0.74054801 0.51498777 0 -0.81149226 0.40922004 0 -0.86963826 0.29699868 
		0 -0.91407013 0.18009347 0 -0.94408602 0.060348082 0 -0.95921302 -0.06034907 0 -0.95921302 
		-0.18009447 0 -0.94408602 -0.29699963 0 -0.91406989 -0.40922081 0 -0.86963809 -0.51498872 
		0 -0.81149215 -0.6126346 0 -0.74054778 -0.70061886 0 -0.65792453 -0.77755386 0 -0.56492621 
		-0.8422268 0 -0.46301821 -0.89361703 0 -0.35380837 -0.93091464 0 -0.23901848 -0.95353115 
		0 -0.12045929 -0.96111 0 -3.426513e-07;
createNode mesh -n "polySurfaceShape1" -p "ring";
	rename -uid "8B77526A-4DD5-74EF-D739-1A9827DE8D17";
	setAttr -k off ".v";
	setAttr ".io" yes;
	setAttr ".vir" yes;
	setAttr ".vif" yes;
	setAttr -s 10 ".gtag";
	setAttr ".gtag[0].gtagnm" -type "string" "bottom";
	setAttr ".gtag[0].gtagcmp" -type "componentList" 0;
	setAttr ".gtag[1].gtagnm" -type "string" "bottomRing";
	setAttr ".gtag[1].gtagcmp" -type "componentList" 0;
	setAttr ".gtag[2].gtagnm" -type "string" "cylBottomCap";
	setAttr ".gtag[2].gtagcmp" -type "componentList" 0;
	setAttr ".gtag[3].gtagnm" -type "string" "cylBottomRing";
	setAttr ".gtag[3].gtagcmp" -type "componentList" 0;
	setAttr ".gtag[4].gtagnm" -type "string" "cylSides";
	setAttr ".gtag[4].gtagcmp" -type "componentList" 1 "vtx[0:49]";
	setAttr ".gtag[5].gtagnm" -type "string" "cylTopCap";
	setAttr ".gtag[5].gtagcmp" -type "componentList" 1 "vtx[0:99]";
	setAttr ".gtag[6].gtagnm" -type "string" "cylTopRing";
	setAttr ".gtag[6].gtagcmp" -type "componentList" 1 "vtx[0:49]";
	setAttr ".gtag[7].gtagnm" -type "string" "sides";
	setAttr ".gtag[7].gtagcmp" -type "componentList" 0;
	setAttr ".gtag[8].gtagnm" -type "string" "top";
	setAttr ".gtag[8].gtagcmp" -type "componentList" 1 "f[0:49]";
	setAttr ".gtag[9].gtagnm" -type "string" "topRing";
	setAttr ".gtag[9].gtagcmp" -type "componentList" 1 "e[0:49]";
	setAttr ".pv" -type "double2" 0.49999998509883881 0.84375 ;
	setAttr ".uvst[0].uvsn" -type "string" "map1";
	setAttr -s 100 ".uvst[0].uvsp[0:99]" -type "float2" 0.65501809 0.82416654
		 0.65134126 0.80489206 0.64527768 0.78623033 0.63692296 0.76847577 0.62640893 0.75190836
		 0.61390138 0.73678935 0.59959751 0.72335714 0.58372295 0.71182358 0.56652796 0.70237064
		 0.54828387 0.69514728 0.52927828 0.69026744 0.50981092 0.68780822 0.4901889 0.68780822
		 0.47072157 0.69026756 0.45171598 0.69514734 0.43347186 0.70237076 0.41627693 0.7118237
		 0.4004024 0.72335726 0.38609853 0.73678946 0.37359098 0.75190854 0.36307698 0.76847601
		 0.35472232 0.78623056 0.3486588 0.80489224 0.344982 0.82416677 0.34374994 0.84375006
		 0.34498203 0.86333334 0.34865883 0.88260788 0.35472238 0.90126956 0.36307704 0.91902411
		 0.37359107 0.93559152 0.38609865 0.95071054 0.40040249 0.9641428 0.41627708 0.9756763
		 0.43347201 0.9851293 0.45171613 0.9923526 0.47072169 0.99723244 0.49018902 0.99969172
		 0.50981104 0.99969172 0.5292784 0.99723244 0.54828393 0.9923526 0.56652808 0.98512924
		 0.58372295 0.97567624 0.59959751 0.96414268 0.61390138 0.95071048 0.62640893 0.93559146
		 0.63692296 0.91902399 0.64527762 0.90126944 0.65134114 0.88260782 0.65501791 0.86333334
		 0.65625 0.84375 0.57750905 0.83395827 0.5756706 0.82432103 0.57263881 0.81499016
		 0.56846148 0.80611289 0.56320447 0.79782915 0.55695069 0.79026967 0.54979873 0.7835536
		 0.54186147 0.77778679 0.53326398 0.77306032 0.52414191 0.76944864 0.51463914 0.76700872
		 0.50490546 0.76577914 0.49509445 0.76577914 0.48536077 0.76700878 0.475858 0.76944864
		 0.46673593 0.77306038 0.45813847 0.77778685 0.45020118 0.7835536 0.44304925 0.79026973
		 0.43679547 0.79782927 0.43153849 0.806113 0.42736116 0.81499028 0.4243294 0.82432115
		 0.42249101 0.83395839 0.42187497 0.84375 0.42249101 0.85354173 0.4243294 0.86317897
		 0.42736119 0.87250978 0.43153852 0.88138705 0.43679553 0.88967073 0.44304931 0.89723027
		 0.45020124 0.9039464 0.45813853 0.90971315 0.46673602 0.91443968 0.47585806 0.9180513
		 0.48536086 0.92049122 0.49509451 0.92172086 0.50490552 0.92172086 0.5146392 0.92049122
		 0.52414197 0.9180513 0.53326404 0.91443962 0.54186147 0.90971315 0.54979879 0.90394634
		 0.55695069 0.89723027 0.56320447 0.88967073 0.56846148 0.881387 0.57263881 0.87250972
		 0.57567054 0.86317891 0.57750899 0.85354167 0.578125 0.84375;
	setAttr ".cuvs" -type "string" "map1";
	setAttr ".dcc" -type "string" "Ambient+Diffuse";
	setAttr ".covm[0]"  0 1 1;
	setAttr ".cdvm[0]"  0 1 1;
	setAttr -s 100 ".vt[0:99]"  3.030697584 0 -0.38286629 2.95881367 0 -0.7596944
		 2.84026766 0 -1.12454164 2.67692876 0 -1.47165418 2.47137308 0 -1.79555786 2.2268424 0 -2.091144562
		 1.94719326 0 -2.35375261 1.63683558 0 -2.57924056 1.30066395 0 -2.76405239 0.94398022 0 -2.90527344
		 0.57240933 0 -3.00067639351 0.19181126 0 -3.048757076 -0.19181177 0 -3.048756838
		 -0.57240981 0 -3.00067615509 -0.94398063 0 -2.90527272 -1.30066419 0 -2.76405168
		 -1.63683558 0 -2.57923961 -1.94719303 0 -2.35375166 -2.22684216 0 -2.091143608 -2.4713726 0 -1.7955569
		 -2.67692804 0 -1.47165334 -2.8402667 0 -1.12454081 -2.95881271 0 -0.75969368 -3.030696392 0 -0.38286573
		 -3.054784298 0 2.9587872e-07 -3.030696154 0 0.38286626 -2.95881248 0 0.75969416 -2.84026623 0 1.12454128
		 -2.67692733 0 1.4716537 -2.47137189 0 1.79555714 -2.22684121 0 2.091143608 -1.94719219 0 2.35375166
		 -1.63683462 0 2.57923937 -1.30066323 0 2.76405096 -0.94397968 0 2.90527177 -0.57240897 0 3.00067472458
		 -0.19181114 0 3.048755407 0.1918117 0 3.048755169 0.57240957 0 3.00067448616 0.94398016 0 2.90527105
		 1.30066347 0 2.76405001 1.63683462 0 2.57923818 1.94719195 0 2.35375047 2.22684097 0 2.091142416
		 2.47137117 0 1.79555607 2.67692637 0 1.47165251 2.8402648 0 1.12454033 2.95881081 0 0.7596935
		 3.030694485 0 0.38286576 3.054782391 0 0 2.62418318 0 -0.33151123 2.56194139 0 -0.65779448
		 2.45929599 0 -0.97370374 2.31786585 0 -1.27425718 2.13988233 0 -1.55471504 1.92815125 0 -1.81065369
		 1.68601167 0 -2.038037539 1.4172833 0 -2.23328042 1.12620294 0 -2.39330316 0.81736225 0 -2.51558208
		 0.49563101 0 -2.59818792 0.16608347 0 -2.63981938 -0.16608323 0 -2.63981938 -0.4956308 0 -2.59818769
		 -0.81736189 0 -2.51558185 -1.1262027 0 -2.39330268 -1.41728246 0 -2.23327994 -1.6860112 0 -2.038037062
		 -1.92814994 0 -1.81065297 -2.13988066 0 -1.55471396 -2.31786489 0 -1.27425671 -2.4592948 0 -0.97370303
		 -2.56193948 0 -0.65779376 -2.62418151 0 -0.33151069 -2.64503813 0 5.5596774e-07 -2.62418127 0 0.3315118
		 -2.56193948 0 0.65779489 -2.45929456 0 0.97370422 -2.31786442 0 1.27425754 -2.13988018 0 1.55471492
		 -1.92814898 0 1.81065369 -1.68601 0 2.0380373 -1.41728151 0 2.23327994 -1.12620187 0 2.39330244
		 -0.81736088 0 2.51558137 -0.49563012 0 2.59818697 -0.16608272 0 2.63981867 0.16608386 0 2.63981867
		 0.49563119 0 2.59818697 0.81736219 0 2.51558089 1.12620258 0 2.39330173 1.41728246 0 2.23327923
		 1.68601096 0 2.038036346 1.9281497 0 1.81065261 2.13988018 0 1.55471396 2.31786418 0 1.27425647
		 2.45929384 0 0.97370332 2.56193852 0 0.6577943 2.62418032 0 0.33151135 2.64503741 0 2.9977599e-07;
	setAttr -s 150 ".ed[0:149]"  0 1 0 1 2 0 2 3 0 3 4 0 4 5 0 5 6 0 6 7 0
		 7 8 0 8 9 0 9 10 0 10 11 0 11 12 0 12 13 0 13 14 0 14 15 0 15 16 0 16 17 0 17 18 0
		 18 19 0 19 20 0 20 21 0 21 22 0 22 23 0 23 24 0 24 25 0 25 26 0 26 27 0 27 28 0 28 29 0
		 29 30 0 30 31 0 31 32 0 32 33 0 33 34 0 34 35 0 35 36 0 36 37 0 37 38 0 38 39 0 39 40 0
		 40 41 0 41 42 0 42 43 0 43 44 0 44 45 0 45 46 0 46 47 0 47 48 0 48 49 0 49 0 0 50 51 0
		 51 52 0 52 53 0 53 54 0 54 55 0 55 56 0 56 57 0 57 58 0 58 59 0 59 60 0 60 61 0 61 62 0
		 62 63 0 63 64 0 64 65 0 65 66 0 66 67 0 67 68 0 68 69 0 69 70 0 70 71 0 71 72 0 72 73 0
		 73 74 0 74 75 0 75 76 0 76 77 0 77 78 0 78 79 0 79 80 0 80 81 0 81 82 0 82 83 0 83 84 0
		 84 85 0 85 86 0 86 87 0 87 88 0 88 89 0 89 90 0 90 91 0 91 92 0 92 93 0 93 94 0 94 95 0
		 95 96 0 96 97 0 97 98 0 98 99 0 99 50 0 0 50 1 1 51 1 2 52 1 3 53 1 4 54 1 5 55 1
		 6 56 1 7 57 1 8 58 1 9 59 1 10 60 1 11 61 1 12 62 1 13 63 1 14 64 1 15 65 1 16 66 1
		 17 67 1 18 68 1 19 69 1 20 70 1 21 71 1 22 72 1 23 73 1 24 74 1 25 75 1 26 76 1 27 77 1
		 28 78 1 29 79 1 30 80 1 31 81 1 32 82 1 33 83 1 34 84 1 35 85 1 36 86 1 37 87 1 38 88 1
		 39 89 1 40 90 1 41 91 1 42 92 1 43 93 1 44 94 1 45 95 1 46 96 1 47 97 1 48 98 1 49 99 1;
	setAttr -s 50 -ch 200 ".fc[0:49]" -type "polyFaces" 
		f 4 0 101 -51 -101
		mu 0 4 48 47 97 98
		f 4 1 102 -52 -102
		mu 0 4 47 46 96 97
		f 4 2 103 -53 -103
		mu 0 4 46 45 95 96
		f 4 3 104 -54 -104
		mu 0 4 45 44 94 95
		f 4 4 105 -55 -105
		mu 0 4 44 43 93 94
		f 4 5 106 -56 -106
		mu 0 4 43 42 92 93
		f 4 6 107 -57 -107
		mu 0 4 42 41 91 92
		f 4 7 108 -58 -108
		mu 0 4 41 40 90 91
		f 4 8 109 -59 -109
		mu 0 4 40 39 89 90
		f 4 9 110 -60 -110
		mu 0 4 39 38 88 89
		f 4 10 111 -61 -111
		mu 0 4 38 37 87 88
		f 4 11 112 -62 -112
		mu 0 4 37 36 86 87
		f 4 12 113 -63 -113
		mu 0 4 36 35 85 86
		f 4 13 114 -64 -114
		mu 0 4 35 34 84 85
		f 4 14 115 -65 -115
		mu 0 4 34 33 83 84
		f 4 15 116 -66 -116
		mu 0 4 33 32 82 83
		f 4 16 117 -67 -117
		mu 0 4 32 31 81 82
		f 4 17 118 -68 -118
		mu 0 4 31 30 80 81
		f 4 18 119 -69 -119
		mu 0 4 30 29 79 80
		f 4 19 120 -70 -120
		mu 0 4 29 28 78 79
		f 4 20 121 -71 -121
		mu 0 4 28 27 77 78
		f 4 21 122 -72 -122
		mu 0 4 27 26 76 77
		f 4 22 123 -73 -123
		mu 0 4 26 25 75 76
		f 4 23 124 -74 -124
		mu 0 4 25 24 74 75
		f 4 24 125 -75 -125
		mu 0 4 24 23 73 74
		f 4 25 126 -76 -126
		mu 0 4 23 22 72 73
		f 4 26 127 -77 -127
		mu 0 4 22 21 71 72
		f 4 27 128 -78 -128
		mu 0 4 21 20 70 71
		f 4 28 129 -79 -129
		mu 0 4 20 19 69 70
		f 4 29 130 -80 -130
		mu 0 4 19 18 68 69
		f 4 30 131 -81 -131
		mu 0 4 18 17 67 68
		f 4 31 132 -82 -132
		mu 0 4 17 16 66 67
		f 4 32 133 -83 -133
		mu 0 4 16 15 65 66
		f 4 33 134 -84 -134
		mu 0 4 15 14 64 65
		f 4 34 135 -85 -135
		mu 0 4 14 13 63 64
		f 4 35 136 -86 -136
		mu 0 4 13 12 62 63
		f 4 36 137 -87 -137
		mu 0 4 12 11 61 62
		f 4 37 138 -88 -138
		mu 0 4 11 10 60 61
		f 4 38 139 -89 -139
		mu 0 4 10 9 59 60
		f 4 39 140 -90 -140
		mu 0 4 9 8 58 59
		f 4 40 141 -91 -141
		mu 0 4 8 7 57 58
		f 4 41 142 -92 -142
		mu 0 4 7 6 56 57
		f 4 42 143 -93 -143
		mu 0 4 6 5 55 56
		f 4 43 144 -94 -144
		mu 0 4 5 4 54 55
		f 4 44 145 -95 -145
		mu 0 4 4 3 53 54
		f 4 45 146 -96 -146
		mu 0 4 3 2 52 53
		f 4 46 147 -97 -147
		mu 0 4 2 1 51 52
		f 4 47 148 -98 -148
		mu 0 4 1 0 50 51
		f 4 48 149 -99 -149
		mu 0 4 0 49 99 50
		f 4 49 100 -100 -150
		mu 0 4 49 48 98 99;
	setAttr ".cd" -type "dataPolyComponent" Index_Data Edge 0 ;
	setAttr ".cvd" -type "dataPolyComponent" Index_Data Vertex 0 ;
	setAttr ".pd[0]" -type "dataPolyComponent" Index_Data UV 0 ;
	setAttr ".hfd" -type "dataPolyComponent" Index_Data Face 0 ;
createNode lightLinker -s -n "lightLinker1";
	rename -uid "B40E65FE-4AA2-1B62-E8B3-3EA6E122D0A3";
	setAttr -s 2 ".lnk";
	setAttr -s 2 ".slnk";
createNode shapeEditorManager -n "shapeEditorManager";
	rename -uid "E3006A08-4BE3-CC63-4178-03AC9C6A8482";
createNode poseInterpolatorManager -n "poseInterpolatorManager";
	rename -uid "1C367FEE-4F7E-AB04-A9B8-0EB92B013783";
createNode displayLayerManager -n "layerManager";
	rename -uid "9040197D-4AE0-F701-691A-F780D81608A3";
createNode displayLayer -n "defaultLayer";
	rename -uid "C05AD162-1A46-E599-8D14-1F83424140AD";
	setAttr ".ufem" -type "stringArray" 0  ;
createNode renderLayerManager -n "renderLayerManager";
	rename -uid "FE5A34CE-44F7-D0CF-7FFF-FC82C187276E";
createNode renderLayer -n "defaultRenderLayer";
	rename -uid "31941F80-E442-EF50-A54C-9FA3E855BE3E";
	setAttr ".g" yes;
createNode aiOptions -s -n "defaultArnoldRenderOptions";
	rename -uid "369B0A96-8345-5F84-C1D7-67919BE78D54";
	setAttr ".version" -type "string" "5.2.0";
createNode aiAOVFilter -s -n "defaultArnoldFilter";
	rename -uid "E1ECCD0D-3B46-2B33-0606-20BF06E8F0A8";
createNode aiAOVDriver -s -n "defaultArnoldDriver";
	rename -uid "83FC2805-974C-3C93-F5C7-31AD7797916D";
createNode aiAOVDriver -s -n "defaultArnoldDisplayDriver";
	rename -uid "CE5AF54D-D44C-CF75-3583-7489F7F42E06";
	setAttr ".output_mode" 0;
	setAttr ".ai_translator" -type "string" "maya";
createNode polyPlatonic -n "polyPlatonic1";
	rename -uid "8E445DC6-7C4A-AECB-B7D6-ECAAC57742B1";
	setAttr ".subdivisionMode" 1;
	setAttr ".subdivisions" 4;
createNode script -n "uiConfigurationScriptNode";
	rename -uid "A9E9B89F-8A41-F1F0-B035-7A9585BFC0E9";
	setAttr ".b" -type "string" (
		"// Maya Mel UI Configuration File.\n//\n//  This script is machine generated.  Edit at your own risk.\n//\n//\n\nglobal string $gMainPane;\nif (`paneLayout -exists $gMainPane`) {\n\n\tglobal int $gUseScenePanelConfig;\n\tint    $useSceneConfig = $gUseScenePanelConfig;\n\tint    $nodeEditorPanelVisible = stringArrayContains(\"nodeEditorPanel1\", `getPanel -vis`);\n\tint    $nodeEditorWorkspaceControlOpen = (`workspaceControl -exists nodeEditorPanel1Window` && `workspaceControl -q -visible nodeEditorPanel1Window`);\n\tint    $menusOkayInPanels = `optionVar -q allowMenusInPanels`;\n\tint    $nVisPanes = `paneLayout -q -nvp $gMainPane`;\n\tint    $nPanes = 0;\n\tstring $editorName;\n\tstring $panelName;\n\tstring $itemFilterName;\n\tstring $panelConfig;\n\n\t//\n\t//  get current state of the UI\n\t//\n\tsceneUIReplacement -update $gMainPane;\n\n\t$panelName = `sceneUIReplacement -getNextPanel \"modelPanel\" (localizedPanelLabel(\"Top View\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tmodelPanel -edit -l (localizedPanelLabel(\"Top View\")) -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\t$editorName = $panelName;\n        modelEditor -e \n            -camera \"|top\" \n            -useInteractiveMode 0\n            -displayLights \"default\" \n            -displayAppearance \"smoothShaded\" \n            -activeOnly 0\n            -ignorePanZoom 0\n            -wireframeOnShaded 0\n            -headsUpDisplay 1\n            -holdOuts 1\n            -selectionHiliteDisplay 1\n            -useDefaultMaterial 0\n            -bufferMode \"double\" \n            -twoSidedLighting 0\n            -backfaceCulling 0\n            -xray 0\n            -jointXray 0\n            -activeComponentsXray 0\n            -displayTextures 0\n            -smoothWireframe 0\n            -lineWidth 1\n            -textureAnisotropic 0\n            -textureHilight 1\n            -textureSampling 2\n            -textureDisplay \"modulate\" \n            -textureMaxSize 16384\n            -fogging 0\n            -fogSource \"fragment\" \n            -fogMode \"linear\" \n            -fogStart 0\n            -fogEnd 100\n            -fogDensity 0.1\n            -fogColor 0.5 0.5 0.5 1 \n"
		+ "            -depthOfFieldPreview 1\n            -maxConstantTransparency 1\n            -rendererName \"vp2Renderer\" \n            -objectFilterShowInHUD 1\n            -isFiltered 0\n            -colorResolution 256 256 \n            -bumpResolution 512 512 \n            -textureCompression 0\n            -transparencyAlgorithm \"frontAndBackCull\" \n            -transpInShadows 0\n            -cullingOverride \"none\" \n            -lowQualityLighting 0\n            -maximumNumHardwareLights 1\n            -occlusionCulling 0\n            -shadingModel 0\n            -useBaseRenderer 0\n            -useReducedRenderer 0\n            -smallObjectCulling 0\n            -smallObjectThreshold -1 \n            -interactiveDisableShadows 0\n            -interactiveBackFaceCull 0\n            -sortTransparent 1\n            -controllers 1\n            -nurbsCurves 1\n            -nurbsSurfaces 1\n            -polymeshes 1\n            -subdivSurfaces 1\n            -planes 1\n            -lights 1\n            -cameras 1\n            -controlVertices 1\n"
		+ "            -hulls 1\n            -grid 1\n            -imagePlane 1\n            -joints 1\n            -ikHandles 1\n            -deformers 1\n            -dynamics 1\n            -particleInstancers 1\n            -fluids 1\n            -hairSystems 1\n            -follicles 1\n            -nCloths 1\n            -nParticles 1\n            -nRigids 1\n            -dynamicConstraints 1\n            -locators 1\n            -manipulators 1\n            -pluginShapes 1\n            -dimensions 1\n            -handles 1\n            -pivots 1\n            -textures 1\n            -strokes 1\n            -motionTrails 1\n            -clipGhosts 1\n            -bluePencil 1\n            -greasePencils 0\n            -excludeObjectPreset \"All\" \n            -shadows 0\n            -captureSequenceNumber -1\n            -width 1\n            -height 1\n            -sceneRenderFilter 0\n            $editorName;\n        modelEditor -e -viewSelected 0 $editorName;\n        modelEditor -e \n            -pluginObjects \"gpuCacheDisplayFilter\" 1 \n            $editorName;\n"
		+ "\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextPanel \"modelPanel\" (localizedPanelLabel(\"Side View\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tmodelPanel -edit -l (localizedPanelLabel(\"Side View\")) -mbv $menusOkayInPanels  $panelName;\n\t\t$editorName = $panelName;\n        modelEditor -e \n            -camera \"|side\" \n            -useInteractiveMode 0\n            -displayLights \"default\" \n            -displayAppearance \"smoothShaded\" \n            -activeOnly 0\n            -ignorePanZoom 0\n            -wireframeOnShaded 0\n            -headsUpDisplay 1\n            -holdOuts 1\n            -selectionHiliteDisplay 1\n            -useDefaultMaterial 0\n            -bufferMode \"double\" \n            -twoSidedLighting 0\n            -backfaceCulling 0\n            -xray 0\n            -jointXray 0\n            -activeComponentsXray 0\n            -displayTextures 0\n            -smoothWireframe 0\n            -lineWidth 1\n            -textureAnisotropic 0\n"
		+ "            -textureHilight 1\n            -textureSampling 2\n            -textureDisplay \"modulate\" \n            -textureMaxSize 16384\n            -fogging 0\n            -fogSource \"fragment\" \n            -fogMode \"linear\" \n            -fogStart 0\n            -fogEnd 100\n            -fogDensity 0.1\n            -fogColor 0.5 0.5 0.5 1 \n            -depthOfFieldPreview 1\n            -maxConstantTransparency 1\n            -rendererName \"vp2Renderer\" \n            -objectFilterShowInHUD 1\n            -isFiltered 0\n            -colorResolution 256 256 \n            -bumpResolution 512 512 \n            -textureCompression 0\n            -transparencyAlgorithm \"frontAndBackCull\" \n            -transpInShadows 0\n            -cullingOverride \"none\" \n            -lowQualityLighting 0\n            -maximumNumHardwareLights 1\n            -occlusionCulling 0\n            -shadingModel 0\n            -useBaseRenderer 0\n            -useReducedRenderer 0\n            -smallObjectCulling 0\n            -smallObjectThreshold -1 \n            -interactiveDisableShadows 0\n"
		+ "            -interactiveBackFaceCull 0\n            -sortTransparent 1\n            -controllers 1\n            -nurbsCurves 1\n            -nurbsSurfaces 1\n            -polymeshes 1\n            -subdivSurfaces 1\n            -planes 1\n            -lights 1\n            -cameras 1\n            -controlVertices 1\n            -hulls 1\n            -grid 1\n            -imagePlane 1\n            -joints 1\n            -ikHandles 1\n            -deformers 1\n            -dynamics 1\n            -particleInstancers 1\n            -fluids 1\n            -hairSystems 1\n            -follicles 1\n            -nCloths 1\n            -nParticles 1\n            -nRigids 1\n            -dynamicConstraints 1\n            -locators 1\n            -manipulators 1\n            -pluginShapes 1\n            -dimensions 1\n            -handles 1\n            -pivots 1\n            -textures 1\n            -strokes 1\n            -motionTrails 1\n            -clipGhosts 1\n            -bluePencil 1\n            -greasePencils 0\n            -excludeObjectPreset \"All\" \n"
		+ "            -shadows 0\n            -captureSequenceNumber -1\n            -width 1\n            -height 1\n            -sceneRenderFilter 0\n            $editorName;\n        modelEditor -e -viewSelected 0 $editorName;\n        modelEditor -e \n            -pluginObjects \"gpuCacheDisplayFilter\" 1 \n            $editorName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextPanel \"modelPanel\" (localizedPanelLabel(\"Front View\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tmodelPanel -edit -l (localizedPanelLabel(\"Front View\")) -mbv $menusOkayInPanels  $panelName;\n\t\t$editorName = $panelName;\n        modelEditor -e \n            -camera \"|front\" \n            -useInteractiveMode 0\n            -displayLights \"default\" \n            -displayAppearance \"smoothShaded\" \n            -activeOnly 0\n            -ignorePanZoom 0\n            -wireframeOnShaded 0\n            -headsUpDisplay 1\n            -holdOuts 1\n            -selectionHiliteDisplay 1\n"
		+ "            -useDefaultMaterial 0\n            -bufferMode \"double\" \n            -twoSidedLighting 0\n            -backfaceCulling 0\n            -xray 0\n            -jointXray 0\n            -activeComponentsXray 0\n            -displayTextures 0\n            -smoothWireframe 0\n            -lineWidth 1\n            -textureAnisotropic 0\n            -textureHilight 1\n            -textureSampling 2\n            -textureDisplay \"modulate\" \n            -textureMaxSize 16384\n            -fogging 0\n            -fogSource \"fragment\" \n            -fogMode \"linear\" \n            -fogStart 0\n            -fogEnd 100\n            -fogDensity 0.1\n            -fogColor 0.5 0.5 0.5 1 \n            -depthOfFieldPreview 1\n            -maxConstantTransparency 1\n            -rendererName \"vp2Renderer\" \n            -objectFilterShowInHUD 1\n            -isFiltered 0\n            -colorResolution 256 256 \n            -bumpResolution 512 512 \n            -textureCompression 0\n            -transparencyAlgorithm \"frontAndBackCull\" \n            -transpInShadows 0\n"
		+ "            -cullingOverride \"none\" \n            -lowQualityLighting 0\n            -maximumNumHardwareLights 1\n            -occlusionCulling 0\n            -shadingModel 0\n            -useBaseRenderer 0\n            -useReducedRenderer 0\n            -smallObjectCulling 0\n            -smallObjectThreshold -1 \n            -interactiveDisableShadows 0\n            -interactiveBackFaceCull 0\n            -sortTransparent 1\n            -controllers 1\n            -nurbsCurves 1\n            -nurbsSurfaces 1\n            -polymeshes 1\n            -subdivSurfaces 1\n            -planes 1\n            -lights 1\n            -cameras 1\n            -controlVertices 1\n            -hulls 1\n            -grid 1\n            -imagePlane 1\n            -joints 1\n            -ikHandles 1\n            -deformers 1\n            -dynamics 1\n            -particleInstancers 1\n            -fluids 1\n            -hairSystems 1\n            -follicles 1\n            -nCloths 1\n            -nParticles 1\n            -nRigids 1\n            -dynamicConstraints 1\n"
		+ "            -locators 1\n            -manipulators 1\n            -pluginShapes 1\n            -dimensions 1\n            -handles 1\n            -pivots 1\n            -textures 1\n            -strokes 1\n            -motionTrails 1\n            -clipGhosts 1\n            -bluePencil 1\n            -greasePencils 0\n            -excludeObjectPreset \"All\" \n            -shadows 0\n            -captureSequenceNumber -1\n            -width 1\n            -height 1\n            -sceneRenderFilter 0\n            $editorName;\n        modelEditor -e -viewSelected 0 $editorName;\n        modelEditor -e \n            -pluginObjects \"gpuCacheDisplayFilter\" 1 \n            $editorName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextPanel \"modelPanel\" (localizedPanelLabel(\"Persp View\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tmodelPanel -edit -l (localizedPanelLabel(\"Persp View\")) -mbv $menusOkayInPanels  $panelName;\n\t\t$editorName = $panelName;\n        modelEditor -e \n"
		+ "            -camera \"|persp\" \n            -useInteractiveMode 0\n            -displayLights \"default\" \n            -displayAppearance \"smoothShaded\" \n            -activeOnly 0\n            -ignorePanZoom 0\n            -wireframeOnShaded 0\n            -headsUpDisplay 1\n            -holdOuts 1\n            -selectionHiliteDisplay 1\n            -useDefaultMaterial 0\n            -bufferMode \"double\" \n            -twoSidedLighting 0\n            -backfaceCulling 0\n            -xray 0\n            -jointXray 0\n            -activeComponentsXray 0\n            -displayTextures 1\n            -smoothWireframe 0\n            -lineWidth 1\n            -textureAnisotropic 0\n            -textureHilight 1\n            -textureSampling 2\n            -textureDisplay \"modulate\" \n            -textureMaxSize 16384\n            -fogging 0\n            -fogSource \"fragment\" \n            -fogMode \"linear\" \n            -fogStart 0\n            -fogEnd 100\n            -fogDensity 0.1\n            -fogColor 0.5 0.5 0.5 1 \n            -depthOfFieldPreview 1\n"
		+ "            -maxConstantTransparency 1\n            -rendererName \"vp2Renderer\" \n            -objectFilterShowInHUD 1\n            -isFiltered 0\n            -colorResolution 256 256 \n            -bumpResolution 512 512 \n            -textureCompression 0\n            -transparencyAlgorithm \"frontAndBackCull\" \n            -transpInShadows 0\n            -cullingOverride \"none\" \n            -lowQualityLighting 0\n            -maximumNumHardwareLights 1\n            -occlusionCulling 0\n            -shadingModel 0\n            -useBaseRenderer 0\n            -useReducedRenderer 0\n            -smallObjectCulling 0\n            -smallObjectThreshold -1 \n            -interactiveDisableShadows 0\n            -interactiveBackFaceCull 0\n            -sortTransparent 1\n            -controllers 1\n            -nurbsCurves 1\n            -nurbsSurfaces 1\n            -polymeshes 1\n            -subdivSurfaces 1\n            -planes 1\n            -lights 1\n            -cameras 1\n            -controlVertices 1\n            -hulls 1\n            -grid 1\n"
		+ "            -imagePlane 1\n            -joints 1\n            -ikHandles 1\n            -deformers 1\n            -dynamics 1\n            -particleInstancers 1\n            -fluids 1\n            -hairSystems 1\n            -follicles 1\n            -nCloths 1\n            -nParticles 1\n            -nRigids 1\n            -dynamicConstraints 1\n            -locators 1\n            -manipulators 1\n            -pluginShapes 1\n            -dimensions 1\n            -handles 1\n            -pivots 1\n            -textures 1\n            -strokes 1\n            -motionTrails 1\n            -clipGhosts 1\n            -bluePencil 1\n            -greasePencils 0\n            -excludeObjectPreset \"All\" \n            -shadows 0\n            -captureSequenceNumber -1\n            -width 1884\n            -height 1203\n            -sceneRenderFilter 0\n            $editorName;\n        modelEditor -e -viewSelected 0 $editorName;\n        modelEditor -e \n            -pluginObjects \"gpuCacheDisplayFilter\" 1 \n            $editorName;\n\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextPanel \"outlinerPanel\" (localizedPanelLabel(\"ToggledOutliner\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\toutlinerPanel -edit -l (localizedPanelLabel(\"ToggledOutliner\")) -mbv $menusOkayInPanels  $panelName;\n\t\t$editorName = $panelName;\n        outlinerEditor -e \n            -showShapes 0\n            -showAssignedMaterials 0\n            -showTimeEditor 1\n            -showReferenceNodes 1\n            -showReferenceMembers 1\n            -showAttributes 0\n            -showConnected 0\n            -showAnimCurvesOnly 0\n            -showMuteInfo 0\n            -organizeByLayer 1\n            -organizeByClip 1\n            -showAnimLayerWeight 1\n            -autoExpandLayers 1\n            -autoExpand 0\n            -showDagOnly 1\n            -showAssets 1\n            -showContainedOnly 1\n            -showPublishedAsConnected 0\n            -showParentContainers 0\n            -showContainerContents 1\n            -ignoreDagHierarchy 0\n"
		+ "            -expandConnections 0\n            -showUpstreamCurves 1\n            -showUnitlessCurves 1\n            -showCompounds 1\n            -showLeafs 1\n            -showNumericAttrsOnly 0\n            -highlightActive 1\n            -autoSelectNewObjects 0\n            -doNotSelectNewObjects 0\n            -dropIsParent 1\n            -transmitFilters 0\n            -setFilter \"defaultSetFilter\" \n            -showSetMembers 1\n            -allowMultiSelection 1\n            -alwaysToggleSelect 0\n            -directSelect 0\n            -isSet 0\n            -isSetMember 0\n            -showUfeItems 1\n            -displayMode \"DAG\" \n            -expandObjects 0\n            -setsIgnoreFilters 1\n            -containersIgnoreFilters 0\n            -editAttrName 0\n            -showAttrValues 0\n            -highlightSecondary 0\n            -showUVAttrsOnly 0\n            -showTextureNodesOnly 0\n            -attrAlphaOrder \"default\" \n            -animLayerFilterOptions \"allAffecting\" \n            -sortOrder \"none\" \n            -longNames 0\n"
		+ "            -niceNames 1\n            -showNamespace 1\n            -showPinIcons 0\n            -mapMotionTrails 0\n            -ignoreHiddenAttribute 0\n            -ignoreOutlinerColor 0\n            -renderFilterVisible 0\n            -renderFilterIndex 0\n            -selectionOrder \"chronological\" \n            -expandAttribute 0\n            $editorName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextPanel \"outlinerPanel\" (localizedPanelLabel(\"Outliner\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\toutlinerPanel -edit -l (localizedPanelLabel(\"Outliner\")) -mbv $menusOkayInPanels  $panelName;\n\t\t$editorName = $panelName;\n        outlinerEditor -e \n            -showShapes 0\n            -showAssignedMaterials 0\n            -showTimeEditor 1\n            -showReferenceNodes 0\n            -showReferenceMembers 0\n            -showAttributes 0\n            -showConnected 0\n            -showAnimCurvesOnly 0\n            -showMuteInfo 0\n"
		+ "            -organizeByLayer 1\n            -organizeByClip 1\n            -showAnimLayerWeight 1\n            -autoExpandLayers 1\n            -autoExpand 0\n            -showDagOnly 1\n            -showAssets 1\n            -showContainedOnly 1\n            -showPublishedAsConnected 0\n            -showParentContainers 0\n            -showContainerContents 1\n            -ignoreDagHierarchy 0\n            -expandConnections 0\n            -showUpstreamCurves 1\n            -showUnitlessCurves 1\n            -showCompounds 1\n            -showLeafs 1\n            -showNumericAttrsOnly 0\n            -highlightActive 1\n            -autoSelectNewObjects 0\n            -doNotSelectNewObjects 0\n            -dropIsParent 1\n            -transmitFilters 0\n            -setFilter \"defaultSetFilter\" \n            -showSetMembers 1\n            -allowMultiSelection 1\n            -alwaysToggleSelect 0\n            -directSelect 0\n            -showUfeItems 1\n            -displayMode \"DAG\" \n            -expandObjects 0\n            -setsIgnoreFilters 1\n"
		+ "            -containersIgnoreFilters 0\n            -editAttrName 0\n            -showAttrValues 0\n            -highlightSecondary 0\n            -showUVAttrsOnly 0\n            -showTextureNodesOnly 0\n            -attrAlphaOrder \"default\" \n            -animLayerFilterOptions \"allAffecting\" \n            -sortOrder \"none\" \n            -longNames 0\n            -niceNames 1\n            -showNamespace 1\n            -showPinIcons 0\n            -mapMotionTrails 0\n            -ignoreHiddenAttribute 0\n            -ignoreOutlinerColor 0\n            -renderFilterVisible 0\n            $editorName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"graphEditor\" (localizedPanelLabel(\"Graph Editor\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Graph Editor\")) -mbv $menusOkayInPanels  $panelName;\n\n\t\t\t$editorName = ($panelName+\"OutlineEd\");\n            outlinerEditor -e \n                -showShapes 1\n"
		+ "                -showAssignedMaterials 0\n                -showTimeEditor 1\n                -showReferenceNodes 0\n                -showReferenceMembers 0\n                -showAttributes 1\n                -showConnected 1\n                -showAnimCurvesOnly 1\n                -showMuteInfo 0\n                -organizeByLayer 1\n                -organizeByClip 1\n                -showAnimLayerWeight 1\n                -autoExpandLayers 1\n                -autoExpand 1\n                -showDagOnly 0\n                -showAssets 1\n                -showContainedOnly 0\n                -showPublishedAsConnected 0\n                -showParentContainers 0\n                -showContainerContents 0\n                -ignoreDagHierarchy 0\n                -expandConnections 1\n                -showUpstreamCurves 1\n                -showUnitlessCurves 1\n                -showCompounds 0\n                -showLeafs 1\n                -showNumericAttrsOnly 1\n                -highlightActive 0\n                -autoSelectNewObjects 1\n                -doNotSelectNewObjects 0\n"
		+ "                -dropIsParent 1\n                -transmitFilters 1\n                -setFilter \"0\" \n                -showSetMembers 0\n                -allowMultiSelection 1\n                -alwaysToggleSelect 0\n                -directSelect 0\n                -showUfeItems 1\n                -displayMode \"DAG\" \n                -expandObjects 0\n                -setsIgnoreFilters 1\n                -containersIgnoreFilters 0\n                -editAttrName 0\n                -showAttrValues 0\n                -highlightSecondary 0\n                -showUVAttrsOnly 0\n                -showTextureNodesOnly 0\n                -attrAlphaOrder \"default\" \n                -animLayerFilterOptions \"allAffecting\" \n                -sortOrder \"none\" \n                -longNames 0\n                -niceNames 1\n                -showNamespace 1\n                -showPinIcons 1\n                -mapMotionTrails 1\n                -ignoreHiddenAttribute 0\n                -ignoreOutlinerColor 0\n                -renderFilterVisible 0\n                $editorName;\n"
		+ "\n\t\t\t$editorName = ($panelName+\"GraphEd\");\n            animCurveEditor -e \n                -displayValues 0\n                -snapTime \"integer\" \n                -snapValue \"none\" \n                -showPlayRangeShades \"on\" \n                -lockPlayRangeShades \"off\" \n                -smoothness \"fine\" \n                -resultSamples 1\n                -resultScreenSamples 0\n                -resultUpdate \"delayed\" \n                -showUpstreamCurves 1\n                -keyMinScale 1\n                -stackedCurvesMin -1\n                -stackedCurvesMax 1\n                -stackedCurvesSpace 0.2\n                -preSelectionHighlight 0\n                -constrainDrag 0\n                -valueLinesToggle 1\n                -outliner \"graphEditor1OutlineEd\" \n                -highlightAffectedCurves 0\n                $editorName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"dopeSheetPanel\" (localizedPanelLabel(\"Dope Sheet\")) `;\n\tif (\"\" != $panelName) {\n"
		+ "\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Dope Sheet\")) -mbv $menusOkayInPanels  $panelName;\n\n\t\t\t$editorName = ($panelName+\"OutlineEd\");\n            outlinerEditor -e \n                -showShapes 1\n                -showAssignedMaterials 0\n                -showTimeEditor 1\n                -showReferenceNodes 0\n                -showReferenceMembers 0\n                -showAttributes 1\n                -showConnected 1\n                -showAnimCurvesOnly 1\n                -showMuteInfo 0\n                -organizeByLayer 1\n                -organizeByClip 1\n                -showAnimLayerWeight 1\n                -autoExpandLayers 1\n                -autoExpand 0\n                -showDagOnly 0\n                -showAssets 1\n                -showContainedOnly 0\n                -showPublishedAsConnected 0\n                -showParentContainers 0\n                -showContainerContents 0\n                -ignoreDagHierarchy 0\n                -expandConnections 1\n                -showUpstreamCurves 1\n"
		+ "                -showUnitlessCurves 0\n                -showCompounds 1\n                -showLeafs 1\n                -showNumericAttrsOnly 1\n                -highlightActive 0\n                -autoSelectNewObjects 0\n                -doNotSelectNewObjects 1\n                -dropIsParent 1\n                -transmitFilters 0\n                -setFilter \"0\" \n                -showSetMembers 0\n                -allowMultiSelection 1\n                -alwaysToggleSelect 0\n                -directSelect 0\n                -showUfeItems 1\n                -displayMode \"DAG\" \n                -expandObjects 0\n                -setsIgnoreFilters 1\n                -containersIgnoreFilters 0\n                -editAttrName 0\n                -showAttrValues 0\n                -highlightSecondary 0\n                -showUVAttrsOnly 0\n                -showTextureNodesOnly 0\n                -attrAlphaOrder \"default\" \n                -animLayerFilterOptions \"allAffecting\" \n                -sortOrder \"none\" \n                -longNames 0\n                -niceNames 1\n"
		+ "                -showNamespace 1\n                -showPinIcons 0\n                -mapMotionTrails 1\n                -ignoreHiddenAttribute 0\n                -ignoreOutlinerColor 0\n                -renderFilterVisible 0\n                $editorName;\n\n\t\t\t$editorName = ($panelName+\"DopeSheetEd\");\n            dopeSheetEditor -e \n                -displayValues 0\n                -snapTime \"integer\" \n                -snapValue \"none\" \n                -outliner \"dopeSheetPanel1OutlineEd\" \n                -showSummary 1\n                -showScene 0\n                -hierarchyBelow 0\n                -showTicks 1\n                -selectionWindow 0 0 0 0 \n                $editorName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"timeEditorPanel\" (localizedPanelLabel(\"Time Editor\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Time Editor\")) -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"clipEditorPanel\" (localizedPanelLabel(\"Trax Editor\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Trax Editor\")) -mbv $menusOkayInPanels  $panelName;\n\n\t\t\t$editorName = clipEditorNameFromPanel($panelName);\n            clipEditor -e \n                -displayValues 0\n                -snapTime \"none\" \n                -snapValue \"none\" \n                -initialized 0\n                -manageSequencer 0 \n                $editorName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"sequenceEditorPanel\" (localizedPanelLabel(\"Camera Sequencer\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Camera Sequencer\")) -mbv $menusOkayInPanels  $panelName;\n\n\t\t\t$editorName = sequenceEditorNameFromPanel($panelName);\n"
		+ "            clipEditor -e \n                -displayValues 0\n                -snapTime \"none\" \n                -snapValue \"none\" \n                -initialized 0\n                -manageSequencer 1 \n                $editorName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"hyperGraphPanel\" (localizedPanelLabel(\"Hypergraph Hierarchy\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Hypergraph Hierarchy\")) -mbv $menusOkayInPanels  $panelName;\n\n\t\t\t$editorName = ($panelName+\"HyperGraphEd\");\n            hyperGraph -e \n                -graphLayoutStyle \"hierarchicalLayout\" \n                -orientation \"horiz\" \n                -mergeConnections 0\n                -zoom 1\n                -animateTransition 0\n                -showRelationships 1\n                -showShapes 0\n                -showDeformers 0\n                -showExpressions 0\n                -showConstraints 0\n"
		+ "                -showConnectionFromSelected 0\n                -showConnectionToSelected 0\n                -showConstraintLabels 0\n                -showUnderworld 0\n                -showInvisible 0\n                -transitionFrames 1\n                -opaqueContainers 0\n                -freeform 0\n                -imagePosition 0 0 \n                -imageScale 1\n                -imageEnabled 0\n                -graphType \"DAG\" \n                -heatMapDisplay 0\n                -updateSelection 1\n                -updateNodeAdded 1\n                -useDrawOverrideColor 0\n                -limitGraphTraversal -1\n                -range 0 0 \n                -iconSize \"smallIcons\" \n                -showCachedConnections 0\n                $editorName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"hyperShadePanel\" (localizedPanelLabel(\"Hypershade\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Hypershade\")) -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"visorPanel\" (localizedPanelLabel(\"Visor\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Visor\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"nodeEditorPanel\" (localizedPanelLabel(\"Node Editor\")) `;\n\tif ($nodeEditorPanelVisible || $nodeEditorWorkspaceControlOpen) {\n\t\tif (\"\" == $panelName) {\n\t\t\tif ($useSceneConfig) {\n\t\t\t\t$panelName = `scriptedPanel -unParent  -type \"nodeEditorPanel\" -l (localizedPanelLabel(\"Node Editor\")) -mbv $menusOkayInPanels `;\n\n\t\t\t$editorName = ($panelName+\"NodeEditorEd\");\n            nodeEditor -e \n                -allAttributes 0\n                -allNodes 0\n                -autoSizeNodes 1\n                -consistentNameSize 1\n                -createNodeCommand \"nodeEdCreateNodeCommand\" \n"
		+ "                -connectNodeOnCreation 0\n                -connectOnDrop 0\n                -copyConnectionsOnPaste 0\n                -connectionStyle \"bezier\" \n                -defaultPinnedState 0\n                -additiveGraphingMode 0\n                -connectedGraphingMode 1\n                -settingsChangedCallback \"nodeEdSyncControls\" \n                -traversalDepthLimit -1\n                -keyPressCommand \"nodeEdKeyPressCommand\" \n                -nodeTitleMode \"name\" \n                -gridSnap 0\n                -gridVisibility 1\n                -crosshairOnEdgeDragging 0\n                -popupMenuScript \"nodeEdBuildPanelMenus\" \n                -showNamespace 1\n                -showShapes 1\n                -showSGShapes 0\n                -showTransforms 1\n                -useAssets 1\n                -syncedSelection 1\n                -extendToShapes 1\n                -showUnitConversions 0\n                -editorMode \"default\" \n                -hasWatchpoint 0\n                $editorName;\n\t\t\t}\n\t\t} else {\n\t\t\t$label = `panel -q -label $panelName`;\n"
		+ "\t\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Node Editor\")) -mbv $menusOkayInPanels  $panelName;\n\n\t\t\t$editorName = ($panelName+\"NodeEditorEd\");\n            nodeEditor -e \n                -allAttributes 0\n                -allNodes 0\n                -autoSizeNodes 1\n                -consistentNameSize 1\n                -createNodeCommand \"nodeEdCreateNodeCommand\" \n                -connectNodeOnCreation 0\n                -connectOnDrop 0\n                -copyConnectionsOnPaste 0\n                -connectionStyle \"bezier\" \n                -defaultPinnedState 0\n                -additiveGraphingMode 0\n                -connectedGraphingMode 1\n                -settingsChangedCallback \"nodeEdSyncControls\" \n                -traversalDepthLimit -1\n                -keyPressCommand \"nodeEdKeyPressCommand\" \n                -nodeTitleMode \"name\" \n                -gridSnap 0\n                -gridVisibility 1\n                -crosshairOnEdgeDragging 0\n                -popupMenuScript \"nodeEdBuildPanelMenus\" \n                -showNamespace 1\n"
		+ "                -showShapes 1\n                -showSGShapes 0\n                -showTransforms 1\n                -useAssets 1\n                -syncedSelection 1\n                -extendToShapes 1\n                -showUnitConversions 0\n                -editorMode \"default\" \n                -hasWatchpoint 0\n                $editorName;\n\t\t\tif (!$useSceneConfig) {\n\t\t\t\tpanel -e -l $label $panelName;\n\t\t\t}\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"createNodePanel\" (localizedPanelLabel(\"Create Node\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Create Node\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"polyTexturePlacementPanel\" (localizedPanelLabel(\"UV Editor\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"UV Editor\")) -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"renderWindowPanel\" (localizedPanelLabel(\"Render View\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Render View\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextPanel \"shapePanel\" (localizedPanelLabel(\"Shape Editor\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tshapePanel -edit -l (localizedPanelLabel(\"Shape Editor\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextPanel \"posePanel\" (localizedPanelLabel(\"Pose Editor\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tposePanel -edit -l (localizedPanelLabel(\"Pose Editor\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n"
		+ "\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"dynRelEdPanel\" (localizedPanelLabel(\"Dynamic Relationships\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Dynamic Relationships\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"relationshipPanel\" (localizedPanelLabel(\"Relationship Editor\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Relationship Editor\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"referenceEditorPanel\" (localizedPanelLabel(\"Reference Editor\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Reference Editor\")) -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"dynPaintScriptedPanelType\" (localizedPanelLabel(\"Paint Effects\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Paint Effects\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"scriptEditorPanel\" (localizedPanelLabel(\"Script Editor\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Script Editor\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"profilerPanel\" (localizedPanelLabel(\"Profiler Tool\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Profiler Tool\")) -mbv $menusOkayInPanels  $panelName;\n"
		+ "\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"contentBrowserPanel\" (localizedPanelLabel(\"Content Browser\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Content Browser\")) -mbv $menusOkayInPanels  $panelName;\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\t$panelName = `sceneUIReplacement -getNextScriptedPanel \"Stereo\" (localizedPanelLabel(\"Stereo\")) `;\n\tif (\"\" != $panelName) {\n\t\t$label = `panel -q -label $panelName`;\n\t\tscriptedPanel -edit -l (localizedPanelLabel(\"Stereo\")) -mbv $menusOkayInPanels  $panelName;\n{ string $editorName = ($panelName+\"Editor\");\n            stereoCameraView -e \n                -camera \"|persp\" \n                -useInteractiveMode 0\n                -displayLights \"default\" \n                -displayAppearance \"wireframe\" \n                -activeOnly 0\n                -ignorePanZoom 0\n                -wireframeOnShaded 0\n"
		+ "                -headsUpDisplay 1\n                -holdOuts 1\n                -selectionHiliteDisplay 1\n                -useDefaultMaterial 0\n                -bufferMode \"double\" \n                -twoSidedLighting 1\n                -backfaceCulling 0\n                -xray 0\n                -jointXray 0\n                -activeComponentsXray 0\n                -displayTextures 0\n                -smoothWireframe 0\n                -lineWidth 1\n                -textureAnisotropic 0\n                -textureHilight 1\n                -textureSampling 2\n                -textureDisplay \"modulate\" \n                -textureMaxSize 16384\n                -fogging 0\n                -fogSource \"fragment\" \n                -fogMode \"linear\" \n                -fogStart 0\n                -fogEnd 100\n                -fogDensity 0.1\n                -fogColor 0.5 0.5 0.5 1 \n                -depthOfFieldPreview 1\n                -maxConstantTransparency 1\n                -objectFilterShowInHUD 1\n                -isFiltered 0\n                -colorResolution 4 4 \n"
		+ "                -bumpResolution 4 4 \n                -textureCompression 0\n                -transparencyAlgorithm \"frontAndBackCull\" \n                -transpInShadows 0\n                -cullingOverride \"none\" \n                -lowQualityLighting 0\n                -maximumNumHardwareLights 0\n                -occlusionCulling 0\n                -shadingModel 0\n                -useBaseRenderer 0\n                -useReducedRenderer 0\n                -smallObjectCulling 0\n                -smallObjectThreshold -1 \n                -interactiveDisableShadows 0\n                -interactiveBackFaceCull 0\n                -sortTransparent 1\n                -controllers 1\n                -nurbsCurves 1\n                -nurbsSurfaces 1\n                -polymeshes 1\n                -subdivSurfaces 1\n                -planes 1\n                -lights 1\n                -cameras 1\n                -controlVertices 1\n                -hulls 1\n                -grid 1\n                -imagePlane 1\n                -joints 1\n                -ikHandles 1\n"
		+ "                -deformers 1\n                -dynamics 1\n                -particleInstancers 1\n                -fluids 1\n                -hairSystems 1\n                -follicles 1\n                -nCloths 1\n                -nParticles 1\n                -nRigids 1\n                -dynamicConstraints 1\n                -locators 1\n                -manipulators 1\n                -pluginShapes 1\n                -dimensions 1\n                -handles 1\n                -pivots 1\n                -textures 1\n                -strokes 1\n                -motionTrails 1\n                -clipGhosts 1\n                -bluePencil 1\n                -greasePencils 0\n                -shadows 0\n                -captureSequenceNumber -1\n                -width 0\n                -height 0\n                -sceneRenderFilter 0\n                -displayMode \"centerEye\" \n                -viewColor 0 0 0 1 \n                -useCustomBackground 1\n                $editorName;\n            stereoCameraView -e -viewSelected 0 $editorName;\n            stereoCameraView -e \n"
		+ "                -pluginObjects \"gpuCacheDisplayFilter\" 1 \n                $editorName; };\n\t\tif (!$useSceneConfig) {\n\t\t\tpanel -e -l $label $panelName;\n\t\t}\n\t}\n\n\n\tif ($useSceneConfig) {\n        string $configName = `getPanel -cwl (localizedPanelLabel(\"Current Layout\"))`;\n        if (\"\" != $configName) {\n\t\t\tpanelConfiguration -edit -label (localizedPanelLabel(\"Current Layout\")) \n\t\t\t\t-userCreated false\n\t\t\t\t-defaultImage \"vacantCell.xP:/\"\n\t\t\t\t-image \"\"\n\t\t\t\t-sc false\n\t\t\t\t-configString \"global string $gMainPane; paneLayout -e -cn \\\"single\\\" -ps 1 100 100 $gMainPane;\"\n\t\t\t\t-removeAllPanels\n\t\t\t\t-ap false\n\t\t\t\t\t(localizedPanelLabel(\"Persp View\")) \n\t\t\t\t\t\"modelPanel\"\n"
		+ "\t\t\t\t\t\"$panelName = `modelPanel -unParent -l (localizedPanelLabel(\\\"Persp View\\\")) -mbv $menusOkayInPanels `;\\n$editorName = $panelName;\\nmodelEditor -e \\n    -cam `findStartUpCamera persp` \\n    -useInteractiveMode 0\\n    -displayLights \\\"default\\\" \\n    -displayAppearance \\\"smoothShaded\\\" \\n    -activeOnly 0\\n    -ignorePanZoom 0\\n    -wireframeOnShaded 0\\n    -headsUpDisplay 1\\n    -holdOuts 1\\n    -selectionHiliteDisplay 1\\n    -useDefaultMaterial 0\\n    -bufferMode \\\"double\\\" \\n    -twoSidedLighting 0\\n    -backfaceCulling 0\\n    -xray 0\\n    -jointXray 0\\n    -activeComponentsXray 0\\n    -displayTextures 1\\n    -smoothWireframe 0\\n    -lineWidth 1\\n    -textureAnisotropic 0\\n    -textureHilight 1\\n    -textureSampling 2\\n    -textureDisplay \\\"modulate\\\" \\n    -textureMaxSize 16384\\n    -fogging 0\\n    -fogSource \\\"fragment\\\" \\n    -fogMode \\\"linear\\\" \\n    -fogStart 0\\n    -fogEnd 100\\n    -fogDensity 0.1\\n    -fogColor 0.5 0.5 0.5 1 \\n    -depthOfFieldPreview 1\\n    -maxConstantTransparency 1\\n    -rendererName \\\"vp2Renderer\\\" \\n    -objectFilterShowInHUD 1\\n    -isFiltered 0\\n    -colorResolution 256 256 \\n    -bumpResolution 512 512 \\n    -textureCompression 0\\n    -transparencyAlgorithm \\\"frontAndBackCull\\\" \\n    -transpInShadows 0\\n    -cullingOverride \\\"none\\\" \\n    -lowQualityLighting 0\\n    -maximumNumHardwareLights 1\\n    -occlusionCulling 0\\n    -shadingModel 0\\n    -useBaseRenderer 0\\n    -useReducedRenderer 0\\n    -smallObjectCulling 0\\n    -smallObjectThreshold -1 \\n    -interactiveDisableShadows 0\\n    -interactiveBackFaceCull 0\\n    -sortTransparent 1\\n    -controllers 1\\n    -nurbsCurves 1\\n    -nurbsSurfaces 1\\n    -polymeshes 1\\n    -subdivSurfaces 1\\n    -planes 1\\n    -lights 1\\n    -cameras 1\\n    -controlVertices 1\\n    -hulls 1\\n    -grid 1\\n    -imagePlane 1\\n    -joints 1\\n    -ikHandles 1\\n    -deformers 1\\n    -dynamics 1\\n    -particleInstancers 1\\n    -fluids 1\\n    -hairSystems 1\\n    -follicles 1\\n    -nCloths 1\\n    -nParticles 1\\n    -nRigids 1\\n    -dynamicConstraints 1\\n    -locators 1\\n    -manipulators 1\\n    -pluginShapes 1\\n    -dimensions 1\\n    -handles 1\\n    -pivots 1\\n    -textures 1\\n    -strokes 1\\n    -motionTrails 1\\n    -clipGhosts 1\\n    -bluePencil 1\\n    -greasePencils 0\\n    -excludeObjectPreset \\\"All\\\" \\n    -shadows 0\\n    -captureSequenceNumber -1\\n    -width 1884\\n    -height 1203\\n    -sceneRenderFilter 0\\n    $editorName;\\nmodelEditor -e -viewSelected 0 $editorName;\\nmodelEditor -e \\n    -pluginObjects \\\"gpuCacheDisplayFilter\\\" 1 \\n    $editorName\"\n"
		+ "\t\t\t\t\t\"modelPanel -edit -l (localizedPanelLabel(\\\"Persp View\\\")) -mbv $menusOkayInPanels  $panelName;\\n$editorName = $panelName;\\nmodelEditor -e \\n    -cam `findStartUpCamera persp` \\n    -useInteractiveMode 0\\n    -displayLights \\\"default\\\" \\n    -displayAppearance \\\"smoothShaded\\\" \\n    -activeOnly 0\\n    -ignorePanZoom 0\\n    -wireframeOnShaded 0\\n    -headsUpDisplay 1\\n    -holdOuts 1\\n    -selectionHiliteDisplay 1\\n    -useDefaultMaterial 0\\n    -bufferMode \\\"double\\\" \\n    -twoSidedLighting 0\\n    -backfaceCulling 0\\n    -xray 0\\n    -jointXray 0\\n    -activeComponentsXray 0\\n    -displayTextures 1\\n    -smoothWireframe 0\\n    -lineWidth 1\\n    -textureAnisotropic 0\\n    -textureHilight 1\\n    -textureSampling 2\\n    -textureDisplay \\\"modulate\\\" \\n    -textureMaxSize 16384\\n    -fogging 0\\n    -fogSource \\\"fragment\\\" \\n    -fogMode \\\"linear\\\" \\n    -fogStart 0\\n    -fogEnd 100\\n    -fogDensity 0.1\\n    -fogColor 0.5 0.5 0.5 1 \\n    -depthOfFieldPreview 1\\n    -maxConstantTransparency 1\\n    -rendererName \\\"vp2Renderer\\\" \\n    -objectFilterShowInHUD 1\\n    -isFiltered 0\\n    -colorResolution 256 256 \\n    -bumpResolution 512 512 \\n    -textureCompression 0\\n    -transparencyAlgorithm \\\"frontAndBackCull\\\" \\n    -transpInShadows 0\\n    -cullingOverride \\\"none\\\" \\n    -lowQualityLighting 0\\n    -maximumNumHardwareLights 1\\n    -occlusionCulling 0\\n    -shadingModel 0\\n    -useBaseRenderer 0\\n    -useReducedRenderer 0\\n    -smallObjectCulling 0\\n    -smallObjectThreshold -1 \\n    -interactiveDisableShadows 0\\n    -interactiveBackFaceCull 0\\n    -sortTransparent 1\\n    -controllers 1\\n    -nurbsCurves 1\\n    -nurbsSurfaces 1\\n    -polymeshes 1\\n    -subdivSurfaces 1\\n    -planes 1\\n    -lights 1\\n    -cameras 1\\n    -controlVertices 1\\n    -hulls 1\\n    -grid 1\\n    -imagePlane 1\\n    -joints 1\\n    -ikHandles 1\\n    -deformers 1\\n    -dynamics 1\\n    -particleInstancers 1\\n    -fluids 1\\n    -hairSystems 1\\n    -follicles 1\\n    -nCloths 1\\n    -nParticles 1\\n    -nRigids 1\\n    -dynamicConstraints 1\\n    -locators 1\\n    -manipulators 1\\n    -pluginShapes 1\\n    -dimensions 1\\n    -handles 1\\n    -pivots 1\\n    -textures 1\\n    -strokes 1\\n    -motionTrails 1\\n    -clipGhosts 1\\n    -bluePencil 1\\n    -greasePencils 0\\n    -excludeObjectPreset \\\"All\\\" \\n    -shadows 0\\n    -captureSequenceNumber -1\\n    -width 1884\\n    -height 1203\\n    -sceneRenderFilter 0\\n    $editorName;\\nmodelEditor -e -viewSelected 0 $editorName;\\nmodelEditor -e \\n    -pluginObjects \\\"gpuCacheDisplayFilter\\\" 1 \\n    $editorName\"\n"
		+ "\t\t\t\t$configName;\n\n            setNamedPanelLayout (localizedPanelLabel(\"Current Layout\"));\n        }\n\n        panelHistory -e -clear mainPanelHistory;\n        sceneUIReplacement -clear;\n\t}\n\n\ngrid -spacing 5 -size 12 -divisions 5 -displayAxes yes -displayGridLines yes -displayDivisionLines yes -displayPerspectiveLabels no -displayOrthographicLabels no -displayAxesBold yes -perspectiveLabelPosition axis -orthographicLabelPosition edge;\nviewManip -drawCompass 0 -compassAngle 0 -frontParameters \"\" -homeParameters \"\" -selectionLockParameters \"\";\n}\n");
	setAttr ".st" 3;
createNode script -n "sceneConfigurationScriptNode";
	rename -uid "CC72FB63-6F45-7E07-38DA-49B710A7BB74";
	setAttr ".b" -type "string" "playbackOptions -min 1 -max 74 -ast 1 -aet 75 ";
	setAttr ".st" 6;
createNode polyTweakUV -n "polyTweakUV1";
	rename -uid "4B76E081-CD4A-EFC1-3299-4696AC12D516";
	setAttr ".uopa" yes;
	setAttr -s 267 ".uvtk";
	setAttr ".uvtk[0:249]" -type "float2" -0.081420898 -0.074105054 -0.069140196
		 -0.087471426 -0.012248375 -0.098666489 -0.018931583 -0.074105054 -0.012248375 -0.049543574
		 -0.069140196 -0.060738623 -0.056859434 -0.074105054 -0.050176263 -0.098666549 -0.0312123
		 -0.087471426 -0.0312123 -0.060738623 -0.050176263 -0.049543574 -0.043493062 -0.074105054
		 -0.00631453 -0.078659862 -0.0072674509 -0.083961844 -0.0085015614 -0.089506328 -0.010121707
		 -0.094585299 -0.079473615 -0.076909721 -0.07712543 -0.080090195 -0.074465632 -0.083204985
		 -0.071714699 -0.085744023 -0.0076077413 -0.099388301 -0.078165352 -0.098221362 -0.073858559
		 -0.095136821 -0.07099092 -0.091225207 -0.080040574 -0.082258284 -0.080804288 -0.088284552
		 -0.077352285 -0.086007893 -0.0060219741 -0.094248176 -0.077674091 -0.092321217 -0.0742172
		 -0.089174449 -0.0079612993 -0.074105054 -0.010763623 -0.074105054 -0.013733126 -0.074105054
		 -0.016535446 -0.074105054 -0.018182218 -0.078659862 -0.017229304 -0.083961844 -0.01599519
		 -0.089506328 -0.014375038 -0.094585299 -0.0090909638 -0.079084009 -0.012248375 -0.079254091
		 -0.010499313 -0.084658027 -0.015405789 -0.079084009 -0.013997436 -0.084658027 -0.012248375
		 -0.090075612 -0.0063145328 -0.069550246 -0.0072674537 -0.064248234 -0.0085015614
		 -0.05870378 -0.010121707 -0.053624794 -0.014375038 -0.053624794 -0.01599519 -0.05870378
		 -0.017229304 -0.064248234 -0.018182218 -0.069550246 -0.0090909638 -0.06912607 -0.010499313
		 -0.063552022 -0.012248375 -0.068955988 -0.012248375 -0.058134511 -0.013997436 -0.063552022
		 -0.015405789 -0.06912607 -0.079473615 -0.071300387 -0.07712543 -0.068119884 -0.074465632
		 -0.065005094 -0.071714699 -0.062466085 -0.07099092 -0.056984901 -0.073858559 -0.053073257
		 -0.078165352 -0.049988702 -0.0076077413 -0.048821785 -0.080040574 -0.065951824 -0.077352285
		 -0.062202215 -0.080804288 -0.059925526 -0.0742172 -0.059035674 -0.077674091 -0.055888876
		 -0.0060219741 -0.053961903 -0.069140196 -0.082679212 -0.069140196 -0.077074558 -0.069140196
		 -0.071135551 -0.069140196 -0.065530896 -0.07712543 -0.074105054 -0.074465632 -0.077272356
		 -0.074465632 -0.070937753 -0.071714699 -0.080283046 -0.071714699 -0.074105054 -0.071714699
		 -0.067927033 -0.067289472 -0.091225207 -0.064421773 -0.095136821 -0.060115039 -0.098221362
		 -0.054816902 -0.099388301 -0.012248375 -0.10345876 -0.012248382 -0.10906345 -0.050176263
		 -0.10906345 -0.050176263 -0.10345876 -0.069140196 -0.096062362 -0.072758496 -0.10095084
		 -0.065521836 -0.10095084 -0.079892039 -0.10402679 -0.069140196 -0.10688388 -0.058388352
		 -0.10402679 -0.016889006 -0.099388301 -0.022187144 -0.098221362 -0.026493922 -0.095136821
		 -0.029361606 -0.091225207 -0.020878881 -0.076909691 -0.023227051 -0.080090195 -0.025886878
		 -0.083204985 -0.028637797 -0.085744023 -0.018474773 -0.094248176 -0.019548193 -0.088284552
		 -0.02267839 -0.092321217 -0.020311907 -0.082258284 -0.023000196 -0.086007893 -0.026135281
		 -0.08917439 -0.020878881 -0.071300387 -0.023227051 -0.068119884 -0.025886878 -0.065005064
		 -0.028637797 -0.062466085 -0.016889006 -0.048821785 -0.022187144 -0.049988702 -0.026493922
		 -0.053073257 -0.029361606 -0.056984901 -0.020311907 -0.065951824 -0.019548193 -0.059925526
		 -0.023000196 -0.062202215 -0.018474773 -0.053961903 -0.02267839 -0.055888876 -0.026135281
		 -0.059035674 -0.012248375 -0.044751316 -0.012248375 -0.039146669 -0.050176263 -0.039146669
		 -0.050176263 -0.044751316 -0.067289472 -0.056984901 -0.064421773 -0.053073257 -0.060115039
		 -0.049988702 -0.054816902 -0.048821785 -0.079892039 -0.044183273 -0.072758496 -0.047259249
		 -0.069140196 -0.041326221 -0.069140196 -0.052147701 -0.065521836 -0.047259249 -0.058388352
		 -0.044183273 -0.066565633 -0.062466085 -0.063814759 -0.065005094 -0.061154902 -0.068119884
		 -0.058806777 -0.071300387 -0.066565633 -0.085744023 -0.063814759 -0.083204985 -0.061154902
		 -0.080090195 -0.058806777 -0.076909721 -0.066565633 -0.067927033 -0.066565633 -0.074105054
		 -0.063814759 -0.070937753 -0.066565633 -0.080283046 -0.063814759 -0.077272356 -0.061154902
		 -0.074105054 -0.052302897 -0.094585299 -0.05392307 -0.089506328 -0.055157185 -0.083961844
		 -0.056110084 -0.078659862 -0.064063132 -0.089174449 -0.060606241 -0.092321217 -0.060928047
		 -0.086007893 -0.056402624 -0.094248176 -0.057476044 -0.088284552 -0.058239758 -0.082258284
		 -0.033063024 -0.091225207 -0.035930693 -0.095136821 -0.040237486 -0.098221362 -0.045535624
		 -0.099388301 -0.020460486 -0.10402679 -0.027594 -0.10095084 -0.0312123 -0.10688388
		 -0.0312123 -0.096062362 -0.03483063 -0.10095084 -0.041964144 -0.10402679 -0.0312123
		 -0.065530896 -0.0312123 -0.071135551 -0.0312123 -0.077074558 -0.0312123 -0.082679212
		 -0.023227051 -0.074105054 -0.025886878 -0.070937753 -0.025886878 -0.077272356 -0.028637797
		 -0.067927033 -0.028637797 -0.074105054 -0.028637797 -0.080283046 -0.045535624 -0.048821785
		 -0.040237486 -0.049988702 -0.035930693 -0.053073257 -0.033063024 -0.056984901 -0.020460486
		 -0.044183273 -0.03121233 -0.041326217 -0.027594 -0.047259249 -0.041964144 -0.044183273
		 -0.03483063 -0.047259249 -0.0312123 -0.052147701 -0.056110084 -0.069550246 -0.055157185
		 -0.064248234 -0.05392307 -0.05870378 -0.052302897 -0.053624794 -0.064063132 -0.059035674
		 -0.060928047 -0.062202215 -0.060606241 -0.055888876 -0.058239758 -0.065951824 -0.057476044
		 -0.059925526 -0.056402624 -0.053961903 -0.054463327 -0.074105054 -0.051661015 -0.074105054
		 -0.048691511 -0.074105054 -0.045889169 -0.074105054 -0.048049569 -0.094585299 -0.046429425
		 -0.089506328 -0.045195341 -0.083961844 -0.044242412 -0.078659862 -0.05333364 -0.079084009
		 -0.051925302 -0.084658027 -0.050176263 -0.079254091 -0.050176263 -0.090075612 -0.048427165
		 -0.084658027 -0.047018856 -0.079084009 -0.033786833 -0.085744023 -0.036537737 -0.083204985
		 -0.039197594 -0.080090195 -0.041545749 -0.076909721 -0.043949842 -0.094248176 -0.039746225
		 -0.092321217 -0.042876422 -0.088284552 -0.036289334 -0.08917439 -0.039424419 -0.086007893
		 -0.042112708 -0.082258284 -0.033786833 -0.062466085 -0.036537737 -0.065005064 -0.039197594
		 -0.068119884 -0.041545749 -0.071300387 -0.033786833 -0.080283046 -0.033786833 -0.074105054
		 -0.036537737 -0.077272356 -0.033786833 -0.067927033 -0.036537737 -0.070937753 -0.039197594
		 -0.074105054 -0.048049569 -0.053624794 -0.046429425 -0.05870378 -0.045195341 -0.064248234
		 -0.044242412 -0.069550246 -0.036289334 -0.059035659 -0.039746225 -0.055888876 -0.039424419
		 -0.062202215 -0.043949842 -0.053961903 -0.042876452 -0.059925526 -0.042112708 -0.065951824
		 -0.050176263 -0.058134511 -0.051925302 -0.063552022 -0.048427165 -0.063552022 -0.05333364
		 -0.06912607;
	setAttr ".uvtk[250:266]" -0.050176263 -0.068955988 -0.047018856 -0.06912607
		 -0.082170308 -0.078659862 -0.083123207 -0.083961844 -0.084357321 -0.089506328 -0.081877708
		 -0.094248176 -0.08346349 -0.099388301 -0.083817065 -0.074105054 -0.082170308 -0.069550246
		 -0.083123207 -0.064248234 -0.084357321 -0.05870378 -0.081877708 -0.053961903 -0.08346349
		 -0.048821785 -0.088104129 -0.10345876 -0.088104129 -0.10906345 -0.088104129 -0.044751316
		 -0.088104129 -0.039146669;
createNode polyMapDel -n "polyMapDel1";
	rename -uid "3255C582-4AB8-8FEE-6FD2-4CA6F0392E9A";
	setAttr ".uopa" yes;
	setAttr ".ics" -type "componentList" 1 "f[*]";
createNode polyTweakUV -n "polyTweakUV2";
	rename -uid "F945A711-4FC0-CB73-A7E4-BC8771048865";
	setAttr ".uopa" yes;
	setAttr -s 200 ".nuv";
	setAttr ".nuv[0:124]"  0 0 0 0 0 1 1 0 0 51
		 1 1 0 50 0 1 1 1 0 0 1 2 1 0 1 52 1 1 1 
		51 0 1 2 2 0 0 2 3 1 0 2 53 1 1 2 52 0 1 
		3 3 0 0 3 4 1 0 3 54 1 1 3 53 0 1 4 4
		 0 0 4 5 1 0 4 55 1 1 4 54 0 1 5 5 0 0 5 
		6 1 0 5 56 1 1 5 55 0 1 6 6 0 0 6 7 1 0 
		6 57 1 1 6 56 0 1 7 7 0 0 7 8 1 0 7 58
		 1 1 7 57 0 1 8 8 0 0 8 9 1 0 8 59 1 1 8 
		58 0 1 9 9 0 0 9 10 1 0 9 60 1 1 9 59 0 1 
		10 10 0 0 10 11 1 0 10 61 1 1 10 60 0 1 11 11
		 0 0 11 12 1 0 11 62 1 1 11 61 0 1 12 12 0 0 12 
		13 1 0 12 63 1 1 12 62 0 1 13 13 0 0 13 14 1 0 
		13 64 1 1 13 63 0 1 14 14 0 0 14 15 1 0 14 65
		 1 1 14 64 0 1 15 15 0 0 15 16 1 0 15 66 1 1 15 
		65 0 1 16 16 0 0 16 17 1 0 16 67 1 1 16 66 0 1 
		17 17 0 0 17 18 1 0 17 68 1 1 17 67 0 1 18 18
		 0 0 18 19 1 0 18 69 1 1 18 68 0 1 19 19 0 0 19 
		20 1 0 19 70 1 1 19 69 0 1 20 20 0 0 20 21 1 0 
		20 71 1 1 20 70 0 1 21 21 0 0 21 22 1 0 21 72
		 1 1 21 71 0 1 22 22 0 0 22 23 1 0 22 73 1 1 22 
		72 0 1 23 23 0 0 23 24 1 0 23 74 1 1 23 73 0 1 
		24 24 0 0 24 25 1 0 24 75 1 1 24 74 0 1 25 25
		 0 0 25 26 1 0 25 76 1 1 25 75 0 1 26 26 0 0 26 
		27 1 0 26 77 1 1 26 76 0 1 27 27 0 0 27 28 1 0 
		27 78 1 1 27 77 0 1 28 28 0 0 28 29 1 0 28 79
		 1 1 28 78 0 1 29 29 0 0 29 30 1 0 29 80 1 1 29 
		79 0 1 30 30 0 0 30 31 1 0 30 81 1 1 30 80 0 1 
		31 31 0 0;
	setAttr ".nuv[125:199]" 31 32 1 0 31 82 1 1 31 81 0
		 1 32 32 0 0 32 33 1 0 32 83 1 1 32 82 0 1 33 
		33 0 0 33 34 1 0 33 84 1 1 33 83 0 1 34 34 0 0 
		34 35 1 0 34 85 1 1 34 84 0 1 35 35 0 0 35 36
		 1 0 35 86 1 1 35 85 0 1 36 36 0 0 36 37 1 0 36 
		87 1 1 36 86 0 1 37 37 0 0 37 38 1 0 37 88 1 1 
		37 87 0 1 38 38 0 0 38 39 1 0 38 89 1 1 38 88
		 0 1 39 39 0 0 39 40 1 0 39 90 1 1 39 89 0 1 40 
		40 0 0 40 41 1 0 40 91 1 1 40 90 0 1 41 41 0 0 
		41 42 1 0 41 92 1 1 41 91 0 1 42 42 0 0 42 43
		 1 0 42 93 1 1 42 92 0 1 43 43 0 0 43 44 1 0 43 
		94 1 1 43 93 0 1 44 44 0 0 44 45 1 0 44 95 1 1 
		44 94 0 1 45 45 0 0 45 46 1 0 45 96 1 1 45 95
		 0 1 46 46 0 0 46 47 1 0 46 97 1 1 46 96 0 1 47 
		47 0 0 47 48 1 0 47 98 1 1 47 97 0 1 48 48 0 0 
		48 49 1 0 48 99 1 1 48 98 0 1 49 49 0 0 49 0
		 1 0 49 50 1 1 49 99 0 1;
select -ne :time1;
	setAttr ".o" 1;
	setAttr ".unw" 1;
select -ne :hardwareRenderingGlobals;
	setAttr ".otfna" -type "stringArray" 22 "NURBS Curves" "NURBS Surfaces" "Polygons" "Subdiv Surface" "Particles" "Particle Instance" "Fluids" "Strokes" "Image Planes" "UI" "Lights" "Cameras" "Locators" "Joints" "IK Handles" "Deformers" "Motion Trails" "Components" "Hair Systems" "Follicles" "Misc. UI" "Ornaments"  ;
	setAttr ".otfva" -type "Int32Array" 22 0 1 1 1 1 1
		 1 1 1 0 0 0 0 0 0 0 0 0
		 0 0 0 0 ;
	setAttr ".dli" 1;
	setAttr ".fprt" yes;
	setAttr ".rtfm" 1;
select -ne :renderPartition;
	setAttr -s 2 ".st";
select -ne :renderGlobalsList1;
select -ne :defaultShaderList1;
	setAttr -s 5 ".s";
select -ne :postProcessList1;
	setAttr -s 2 ".p";
select -ne :defaultRenderingList1;
select -ne :standardSurface1;
	setAttr ".bc" -type "float3" 0.40000001 0.40000001 0.40000001 ;
	setAttr ".sr" 0.40000000596046448;
select -ne :initialShadingGroup;
	setAttr -s 2 ".dsm";
	setAttr ".ro" yes;
select -ne :initialParticleSE;
	setAttr ".ro" yes;
select -ne :defaultRenderGlobals;
	addAttr -ci true -h true -sn "dss" -ln "defaultSurfaceShader" -dt "string";
	setAttr ".ren" -type "string" "mayaHardware2";
	setAttr ".dss" -type "string" "lambert1";
select -ne :defaultResolution;
	setAttr ".pa" 1;
select -ne :defaultColorMgtGlobals;
	setAttr ".cfe" yes;
	setAttr ".cfp" -type "string" "<MAYA_RESOURCES>/OCIO-configs/Maya2022-default/config.ocio";
	setAttr ".vtn" -type "string" "ACES 1.0 SDR-video (sRGB)";
	setAttr ".vn" -type "string" "ACES 1.0 SDR-video";
	setAttr ".dn" -type "string" "sRGB";
	setAttr ".wsn" -type "string" "ACEScg";
	setAttr ".otn" -type "string" "ACES 1.0 SDR-video (sRGB)";
	setAttr ".potn" -type "string" "ACES 1.0 SDR-video (sRGB)";
select -ne :hardwareRenderGlobals;
	setAttr ".ctrs" 256;
	setAttr ".btrs" 512;
select -ne :ikSystem;
	setAttr -s 4 ".sol";
connectAttr "polyTweakUV1.out" "planetShape.i";
connectAttr "polyTweakUV1.uvtk[0]" "planetShape.uvst[0].uvtw";
connectAttr "polyTweakUV2.out" "ringShape.i";
connectAttr "polyTweakUV2.uvtk[0]" "ringShape.uvst[0].uvtw";
relationship "link" ":lightLinker1" ":initialShadingGroup.message" ":defaultLightSet.message";
relationship "link" ":lightLinker1" ":initialParticleSE.message" ":defaultLightSet.message";
relationship "shadowLink" ":lightLinker1" ":initialShadingGroup.message" ":defaultLightSet.message";
relationship "shadowLink" ":lightLinker1" ":initialParticleSE.message" ":defaultLightSet.message";
connectAttr "layerManager.dli[0]" "defaultLayer.id";
connectAttr "renderLayerManager.rlmi[0]" "defaultRenderLayer.rlid";
connectAttr ":defaultArnoldDisplayDriver.msg" ":defaultArnoldRenderOptions.drivers"
		 -na;
connectAttr ":defaultArnoldFilter.msg" ":defaultArnoldRenderOptions.filt";
connectAttr ":defaultArnoldDriver.msg" ":defaultArnoldRenderOptions.drvr";
connectAttr "polyPlatonic1.output" "polyTweakUV1.ip";
connectAttr "polySurfaceShape1.o" "polyMapDel1.ip";
connectAttr "polyMapDel1.out" "polyTweakUV2.ip";
connectAttr "defaultRenderLayer.msg" ":defaultRenderingList1.r" -na;
connectAttr "planetShape.iog" ":initialShadingGroup.dsm" -na;
connectAttr "ringShape.iog" ":initialShadingGroup.dsm" -na;
// End of planet.ma
