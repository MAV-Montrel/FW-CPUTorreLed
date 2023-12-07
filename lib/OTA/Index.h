const char serverIndex[] PROGMEM = R"(
<!DOCTYPE html>
<html>
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <title>Firmware Update</title>
  <!-- Carrega os estilos do Bootstrap e jQuery -->
  <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css"> 
  <script src="https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.16.0/umd/popper.min.js"></script>
  <script src='https://code.jquery.com/jquery-3.6.0.min.js'></script>
  <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js"></script>
  <script src="https://maxcdn.bootstrapcdn.com/bootstrap/4.5.2/js/bootstrap.min.js"></script> 

  <script>
         
        function updateDebugLabel() {
             activeAjaxRequest = null;    
          $.ajax({
          url: '/getDebugOutput', // Rota no servidor para obter o conteúdo de debug
          method: 'GET',
          success: function(response) {
            response = response.replace(/\n/g, '<br>');
              // Limpar mensagem de erro anterior
            $('#error-message').text('');
                          
            $('#debugBox').html(response);

            var debugMessages = document.getElementById('debugBox');
            debugMessages.scrollTop = debugMessages.scrollHeight;                          

            // Limpar a referência à solicitação após a conclusão bem-sucedida
            activeAjaxRequest = null;        
                         
          },
          error: function() {
            console.error('Erro ao buscar conteúdo de debug.');

            // Exibir mensagem de erro para o usuário
            $('#error-message').text('Erro de conexão. Verifique sua rede.');

            // Limpar a referência à solicitação após a conclusão bem-sucedida
            activeAjaxRequest = null;  
          }  
        });
      }

       // Função para limpar o conteúdo da debugBox
        function clearDebugBox() {
          $.ajax({
            url: '/clearDebugData', // Substitua pela rota correta no servidor
            method: 'GET', // Use o método HTTP apropriado (GET, POST, etc.)
            success: function(response) {
              // Limpa o conteúdo da div #debugMessages após o sucesso da chamada
              $('#debugMessages').html('');
              $('#debugBox').html('');
              console.log('Debug limpo com sucesso no servidor.');
            },
            error: function() {
              console.error('Erro ao limpar o debug no servidor.');
              // Exibir mensagem de erro para o usuário
              $('#error-message').text('Erro de conexão. Verifique sua rede.');
            }
          });
        }     

        function limpaHTML() {
           clearDebugBox();      
           console.log('Debug limpo com sucesso no servidor.');
        }

      // Chama a função a cada 5 segundos (5000 milissegundos)
      setInterval(updateDebugLabel, 1000);

    // Função para verificar a senha
    function checkPassword() {
      var enteredPassword = $('#modalPassword').val();
      
      // Verifica se a senha está correta (substitua '1234' pela senha real)
      if (enteredPassword === '1234') {
        console.log('Senha correta. Iniciando atualização...');
        $('#prg').html('AGUARDE CARREGANDO !!!');
        $('#passwordModal').modal('hide'); // Fecha o modal
        var form = $('#upload_form')[0];
        var data = new FormData(form);
        $.ajax({
          url: '/update',
          type: 'POST',
          data: data,
          contentType: false,
          processData: false,
          xhr: function() {
            var xhr = new window.XMLHttpRequest();
            xhr.upload.addEventListener('progress', function(evt) {
              if (evt.lengthComputable) {
                var per = evt.loaded / evt.total;
                //$('#prg').html('progress: ' + Math.round(per*100) + '%');
                if (per < 1) {
                  $('#prg').html('progress: ' + Math.round(per * 100) + '%');
                } else {
                  $('#prg').html('COMPLETADO 100 %  AGUARDANDO REINICIAR !!!');
                }
              }
            }, false);
            return xhr;
          },
          success: function(d, s) {
            console.log('Atualização bem-sucedida!');

             $('#prg').html('AGUARDE REINICIANDO !!!');
            //window.location.href = '/serverIndex';
          },
          error: function (a, b, c) {
            console.error('Erro na atualização:', a, b, c);
            //window.location.href = '/serverIndex';
          }
        });
      } else {
        alert('Senha incorreta. Tente novamente.');
        $('#modalPassword').val('');
      }
    }

    // Função para redirecionar
    function redirectToServerIndex() {    
      window.location.href = '/serverIndex';
    }

     // Função testeHardware que envia o número para a rota
      function testeHardware(number) {
        // Suponha que você esteja usando o jQuery para fazer a solicitação AJAX
        $.ajax({
          url: '/testeHardware', // Substitua pela rota correta no servidor
          method: 'PUT', // Use o método HTTP PUT
          data: JSON.stringify({ number: number }), // Envie o número como JSON
          contentType: 'application/json', // Define o tipo de conteúdo para JSON
          success: function(response) {
            // Lida com a resposta do servidor, se necessário
            console.log('Número enviado com sucesso para a rota.');
          },
          error: function() {
            console.error('Erro ao enviar o número para a rota.');
          }
        });
      } 

    // Função que será executada após o carregamento completo da página
    document.addEventListener('DOMContentLoaded', function() {
      // Adicione um ouvinte de eventos para o botão
      document.getElementById('clearDebugButton').addEventListener('click', clearDebugBox);
     });
  </script>
</head>
<body>
  <div class="container mt-4">
    <h1 class="text-center">Firmware Update</h1>    
    <form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>
    
      <div class="form-group">
        <label for='update'>Select Firmware File:</label>

        <!-- Mantém o estilo padrão do botão "Escolher arquivo" e define o atributo "accept" -->
        <input type='file' id='update' name='update' class="form-control-file" required accept=".bin">
      </div>
      <button type='button' class='btn btn-primary' data-toggle='modal' data-target='#passwordModal'>Update</button>
    </form>    
    <div id='prg' class="mt-3">progress: 0%</div>
    
    <div id="error-message" style="color: red; font-weight: bold;"></div>
    <div style="margin-top: 1em;" id='debugLabel'>Console debug:</div>
    <div id='debugBox' class="mt-4" style="border: 1px solid #ccc; max-height: 10em; height: 10em; overflow-y: scroll; padding: 10px; background-color: black; color: white;">
    </div>

    <div style="margin-top: 10px;"></div>
    <button id="clearDebugButton" style="background-color: blue; color: white;">Limpar Debug</button>
    <div></div>

    <div class='modal' id='passwordModal'>
      <div class='modal-dialog'>
        <div class='modal-content'>
          <div class='modal-header'>
            <h4 class='modal-title'>Digite a senha:</h4>
            <button type='button' class='close' data-dismiss='modal'>&times;</button>
          </div>
           <form id="passwordForm">
            <div class='modal-body'>
              <input type='password' id='modalPassword' class='form-control' placeholder='Senha' autocomplete='new-password'>
            </div>
            <div class='modal-footer'>
              <button type='button' class='btn btn-primary' onclick='checkPassword()'>Confirmar</button>
              <button type='button' class='btn btn-secondary' data-dismiss='modal'>Fechar</button>
            </div>
          </form>
        </div>
      </div>
    </div>
  </div>
</body>
</html>

)";