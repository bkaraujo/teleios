import os
import glob
import argparse
import re

def remover_comentarios_documentacao(conteudo):
    """
    Remove comentários de documentação do tipo /** ... */ do conteúdo
    """
    # Padrão para encontrar comentários de documentação /** ... */
    padrao = r'/\*\*.*?\*/'
    return re.sub(padrao, '', conteudo, flags=re.DOTALL)

def coletar_arquivos(diretorio_base, extensoes, ignorar_dirs):
    """
    Coleta arquivos com as extensões especificadas, ignorando diretórios na lista
    """
    arquivos_encontrados = []

    for extensao in extensoes:
        # Busca recursiva por arquivos com a extensão
        padrao = os.path.join(diretorio_base, '**', f'*{extensao}')
        arquivos = glob.glob(padrao, recursive=True)

        for arquivo in arquivos:
            # Verifica se o arquivo está em um diretório que deve ser ignorado
            deve_ignorar = False
            caminho_relativo = os.path.relpath(arquivo, diretorio_base)

            for dir_ignorar in ignorar_dirs:
                # Verifica se algum dos diretórios ignorados está no caminho
                partes_caminho = caminho_relativo.split(os.sep)
                if any(parte == dir_ignorar for parte in partes_caminho):
                    deve_ignorar = True
                    break

            if not deve_ignorar:
                arquivos_encontrados.append(arquivo)

    return arquivos_encontrados

def agrupar_arquivos(diretorio_base, extensao, arquivo_saida, ignorar_dirs, remover_docs):
    """
    Agrupa arquivos com extensão específica em um único arquivo de saída
    """
    arquivos = coletar_arquivos(diretorio_base, [extensao], ignorar_dirs)

    if not arquivos:
        print(f"Nenhum arquivo {extensao} encontrado em {diretorio_base}")
        return 0

    with open(arquivo_saida, 'w', encoding='utf-8') as saida:
        for caminho_arquivo in arquivos:
            # Calcula o caminho relativo para exibir no comentário
            caminho_relativo = os.path.relpath(caminho_arquivo, diretorio_base)

            # Escreve o caminho do arquivo como comentário
            saida.write(f"// {caminho_relativo}\n")

            try:
                with open(caminho_arquivo, 'r', encoding='utf-8') as entrada:
                    conteudo = entrada.read()

                    # Remove comentários de documentação se solicitado
                    if remover_docs:
                        conteudo = remover_comentarios_documentacao(conteudo)

                    saida.write(conteudo)

                # Adiciona separador entre arquivos
                saida.write("\n\n// " + "="*50 + "\n\n")
                print(f"Processado: {caminho_relativo}")

            except Exception as e:
                print(f"Erro ao processar {caminho_relativo}: {e}")

    return len(arquivos)

def main():
    """
    Função principal que executa o agrupamento dos arquivos
    """
    parser = argparse.ArgumentParser(
        description='Agrupa arquivos C/CPP em arquivos consolidados para IA',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Exemplos de uso:
  # Agrupar arquivos do diretório atual
  python agrupar_c.py 
  
  # Agrupar arquivos de um diretório específico
  python agrupar_c.py engine/src/main --ignore glad stb
  
  # Agrupar mantendo comentários de documentação
  python agrupar_c.py engine/src/main --manter-docs --ignore glad stb
  
  # Agrupar ignorando múltiplos diretórios
  python agrupar_c.py engine/src/main --ignore build .git docs test glad stb
  
  # Especificar arquivos de saída personalizados
  python agrupar_c.py engine/src/main -c meus_codigos.c -H meus_headers.h --ignore glad stb
        '''
    )

    parser.add_argument(
        'diretorio',
        nargs='?',
        default='.',
        help='Diretório raiz para busca (padrão: diretório atual)'
    )

    parser.add_argument(
        '--ignore', '-i',
        nargs='+',
        default=['build', '.git', 'node_modules', '__pycache__'],
        help='Diretórios para ignorar (padrão: build .git node_modules __pycache__)'
    )

    parser.add_argument(
        '--manter-docs', '-m',
        action='store_true',
        help='Manter comentários de documentação /** ... */ (padrão: remover)'
    )

    parser.add_argument(
        '--arquivo-c', '-c',
        default='consolidado.c',
        help='Nome do arquivo de saída para código C (padrão: consolidado.c)'
    )

    parser.add_argument(
        '--arquivo-h', '-H',
        dest='arquivo_h',
        default='consolidado.h',
        help='Nome do arquivo de saída para headers (padrão: consolidado.h)'
    )

    args = parser.parse_args()

    # Resolve o caminho absoluto do diretório base
    diretorio_base = os.path.abspath(args.diretorio)

    # Verifica se o diretório existe
    if not os.path.isdir(diretorio_base):
        print(f"Erro: Diretório '{args.diretorio}' não encontrado.")
        print(f"Caminho absoluto tentado: {diretorio_base}")
        print(f"Diretório atual de trabalho: {os.getcwd()}")
        return

    print(f"Diretório base: {diretorio_base}")
    print(f"Diretório atual: {os.getcwd()}")
    print(f"Ignorando diretórios: {', '.join(args.ignore)}")
    print(f"Remover documentação: {not args.manter_docs}")
    print("-" * 50)

    # Agrupa arquivos .c
    count_c = agrupar_arquivos(
        diretorio_base,
        '.c',
        args.arquivo_c,
        args.ignore,
        not args.manter_docs  # Remove docs se não for para manter
    )

    # Agrupa arquivos .h
    count_h = agrupar_arquivos(
        diretorio_base,
        '.h',
        args.arquivo_h,
        args.ignore,
        not args.manter_docs  # Remove docs se não for para manter
    )

    print("-" * 50)
    print(f"Processamento concluído!")
    print(f"Arquivos .c processados: {count_c} -> {args.arquivo_c}")
    print(f"Arquivos .h processados: {count_h} -> {args.arquivo_h}")

if __name__ == "__main__":
    main()