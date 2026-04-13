我正在構思一個程式語言，基於C/C++。
其編譯器編譯一個檔案時，會將檔案以"行", "單詞"的方式，切成一個二維list。每一行都是一個list，用lisp的方式處理。
本質上這是前處理器，會先將檔案編譯成一個C++原始碼，具體方式如下：
在解釋器執行時，第一行這個list會先被執行，然後依序。若執行的list有返回字串，那就append到生成的C++原始碼中。
單詞就是symbol。若要表示字串，則用""包裹。若要表示數字或其他東西，那就是用'作為前綴，比如'int'123, 'bool'true, 'float'3.14。
編譯時，編譯器會從環境中的dict取出symbol相應的值，可能是字串、數字、其他資料結構。
編譯時，整個環境就是一個dict，就像是python與lua一樣。

然後是一些特殊字元：小/中/大括號()[]{}。反斜線。"'。註解//與/**/。
其優先順序為反斜線>註解>">'>.>小括號>大括號>中括號>單詞。
其中小中大括號除非有用反斜線修飾，否則會被當作單詞切出來。也就是(word)會被切成"(", "word", ")"。
反斜線則與C語言一樣。並且若放在一行末尾，則下一行也會被串在這一行的後面，但會加上一個空格。
比如line1\
line2
會變成line1 line2
關於quotation mark:'，這個的使用形式類似lisp，但我要擴充一下它的功能。
	單純的'A，那就是一個(quote A)。
	但若是'A'B，那就是(A B)。
	若是'A'B'C，那就是(A (B C))。
	若是'A'B'C'D，那就是(A (B (C D)))。
	若是A'，那就是(A)。
	若是A'B'，那就是(A B)。
	若是A'B'C'，那就是((A B) C)。
	若是A'B'C'D'，那就是(((A B) C) D)。
	依據最前面與最後面有無'，來決定是前向嵌套還是後向嵌套。
	若最前面與最後面都沒有'，那就是直接連起來成list。
	比如A'B，那就是(A B)
	若是A'B'C，那就是(A B C)。
	若是前後都有，那就當成前後都沒有。
	'A'B'C'，那就是(A B C)。
	'A'B'，那就是(A B)。
	連續的''視為一個'。比如''A，視為'A。
	其中的A,B,C,D，可以是symbol或被括號包住的資料結構。
	可以用反斜線進行跳脫，比如
	ex. \'atom -> 'atom。這是一個包含quotation mark的symbol。
	ex. atom\ haha -> atom haha，這是一個包含空格的symbol。
而 double quotation marks: "，它的使用方式只有一種，那就是把包起來的東西"XXX"轉成：(str XXX)。優先級僅次於註解。
	後面可以不用，比如"haha等同於"haha"。
	ex. "hello world"
	ex. "hello\" world\n"
關於數字型式的表示法，那就是在此基礎上做擴充，比如'int32'123，那其實就是(int32 123)

每一行都是一個list，除非被括號包住，那就依據包住它的括號種類來判它它是屬於哪種資料結構。
被括號包住的單詞(無論行數,大中小括號)，都同屬一行。
被小括號包住的，會被變成list。
被中括號包住的，會被變成array。
被大括號包住的，會被變成dict或set。表述方式類似python。是dict還是set，取決於有沒有給value。
對於list，會像是lisp語言一樣，從環境中找到第一個symbol相應的函數，並對整個list進行運算。
對於array，那就像是C語言的int[]一樣，就是一個陣列，一個連續的記憶體。
對於dict，表述形式類似{"key_a":1, "key_b":2}。並沒有規定key或value必須是相同型別。只是若key或value是相同型別，其內部實現會是比較高效的相應資料結構。
對於set，表述形式類似{"val_1", "val_2"}，與dict一樣，不限制相同型別，但最好相同。

其實，中括號是語法糖，其本質是[a, b,c] -> (arr a b c)
大括號也是語法糖，其本質是{a:'int'1, b: 'int'2, c :'int'3, d : 'int'4} -> (dict '(a 'int'1) '(b 'int'2) '(c 'int'3) '(d 'int'4))
	而{a, b,c}，則會變成(set a b c)
大括號與中括號的空格不代表什麼，而是用來分隔的。

整個的流程會是
	Lexer (詞法分析)
		處理轉義字元 \、註解 // /**/、字串 " 與引號 '。
		根據括號深度判定「邏輯行」。
		將原始碼切分為 Token 流。
	Parser (語法解析 - AST 生成)
		將 Token 流轉換為list<symbol> (AST)。
		語法糖轉換：
		[...] -> (arr ...)：在中括號內，僅以 , 作為元素分隔符，空格不具分隔功能。
		{k:v, ...} -> (dict '(k v) ...)：在大括號內，僅以 : 分隔鍵值、, 分隔對項，空格不具分隔功能。
		{v1, v2, ...} -> (set v1 v2 ...)：在大括號內，僅以 , 作為元素分隔符，空格不具分隔功能。
		引號擴展 (')：採遞迴巢狀結構。
	Evaluator (求值 - Runtime)
		依照lisp的方法，list的第一個symbol，會去找有沒有相應的函數/macro，有的話就執行。
		執行後，這個list就會變成一個值，可能是array, list, dict, set, int, function等。也可能是一個普通的symbol或list<symbol>, list<...>，假如用quote的話。
		執行期間，list中的每個symbol都會進行求值，具體行為就像這樣：
			(add var1 var2)，那麼會先對var1,var2進行求值，會先從環境中找到"var1", "var2"對應的值：1, 2
			然後再執行(add 1 2)。
		最終，因為整個檔案都是一個list，所以會回傳一個值。
		核心系統函數：
			get：根據參數型別從環境、List 或 Dict 中取出值但不對結果再求值。
				(get symbol) -> _env["symbol"] // 若是symbol，自動轉成字串來取值。
				(get 3) -> _env[3] // 就像是lua一樣，從環境table取值
				(get "val") -> _env["val"] // 就像是lua一樣，用字串從環境table取值
				(get some_list 3) -> some_list[3]
				(get some_dict "key") -> some_dict["key"]
			exec：顯式對一個 Value 進行求值。可以是symbol, list, dict。
			quote：返回其參數本身而不求值。
			def：定義環境變數，如 (def var 123)，這等於lua的_env["var"]=123。
			make：統一的資料結構構造函數。
			(make "int" 123) -> 返回數值 123。
			(make "std::list" 1 2 3) -> 將參數轉為 C++ std::list。
			所有容器（dict, list, arr）本質上皆透過 make 實作。

get這個系統函數還有一個有趣的地方，他會隨著其參數的型別而改變行為。
比如(get 'haha)，'haha是一個symbol，那他就會自動變成執行(get "haha")。從環境中取出相應的值。然後不求值。
若haha是一個list或array，那就是(get haha 'int'1)，那就是返回第一個元素。並且對該元素不求值。
若haha是一個dict，那就是(get haha "key1")，那就是返回dict["key_1"]，並且對該value不求值。
所以要注意，(get haha)之後，若haha是list，則其不會被執行。還需要額外加上exec如(exec (get 'haha))。

函數分兩種，一種是系統函數，也就是最基礎的def, if/else, 加減乘除, 比較...等。
系統函數在環境中直接對應到特定的symbol，這就是會形成所謂關鍵字。(包括int32那種)

另一種是自定義函數，內部就是dict，就是像lua一樣。
剩下的，對於list操作那就是直接抄lisp。若第一個symbol是quote，那就是直接傳回後面的list，否則求值。
對於dict操作那就是抄lua，若其_metatable中有_exec在，那就執行。否則就是普通dict。
注意以下範例：
	ex. (print (+ 1 2)) -> 3
	ex. (print '(+ 1 2)) -> (+ 1 2)
	ex. (let ((l '(+ 1 2))) (print l)) -> (+ 1 2) // 沒有加上exec，那就不執行
	ex. (let ((l '(+ 1 2))) (print (exec l))) -> 3

這個語言可以用C++來實作，並且各種資料結構直接使用std。
然後array還是先限制說必須是相同型別；dict的key必須是相同型別，value若不是相同型別則使用std::any；set必須是相同型別。list則使用std::any。

然後def這東西，形式應該是(def "var" "value"), (def "var2" 'int'13)。本質上是環境table["var"] = 13。相應的get也是
有一些特殊的東西是需要先實現的，比如(to_symbol "haha")，會返回一個symbol:haha。
還有(split_special_symbol aa.bb)，會返回(aa . bb)。
還有(split_by_symbol . aa.bb)，會返回(aa bb)。

編譯成binary時，會變成json那樣的感覺。可以是bson，或可讀的文字。
最好是能直接編譯成C++原始碼。

然後是def, def_func與def_macro。那就是照抄lisp。

最好先實現macro。

關於資料結構，有一個系統函數叫做make。其使用方法如下：
	(make "int" 123)，這會返回一個值123。
	(make "std::list" 1 2 3)，這會嘗試將後面的參數弄成對應的C++資料結構，然後返回。
其原理是這樣的：
	編譯器本身已經註冊了資料結構為"std::list"，並實作了相應的從list變成這個資料結構的函數。
基本上，那些dict, list, arr, int，全部都是用這個。